// 创建新的 tile_catalog.h 文件，内容如下：

#ifndef TILE_CATALOG_H
#define TILE_CATALOG_H

#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <map>
#include <vector>
#include <regex>

// 功能: 扫描目录中的 Tile_+xxx_+yyy(.osgb / _Lnn_code.osgb) 文件, 建立索引, 提供坐标 & 层级 文件查询功能
// 用于渲染/调度 (对应LOD细节层次)
namespace TileCatalog {

struct TileRecord {
    int x = 0;
    int y = 0;
    QString baseFile;                               // Tile_+000_+000.osgb (基础层)
    std::map<int, std::vector<QString>> levelFiles; // L -> 文件列表
};

using TileKey = std::pair<int,int>;
using TileMap = std::map<TileKey, TileRecord>;

struct ScanStats { int tiles=0; int levels=0; int files=0; };

inline int parseSignedTriple(const QString& s) {
    if(s.size()!=4) return 0; // +000
    bool ok=false; int v = s.mid(1).toInt(&ok); if(!ok) return 0; return (s[0]=='-'?-v:v);
}

// 扫描根目录 (例如 .../Production_3/Data) 中的 Tile_* 目录
inline ScanStats scan(const QString& rootDir, TileMap& out) {
    out.clear();
    QDir base(rootDir);
    ScanStats st; if(!base.exists()){ qWarning() << "TileCatalog: rootDir not exist" << rootDir; return st; }
    QStringList dirNames = base.entryList(QStringList() << "Tile_*", QDir::Dirs|QDir::NoDotAndDotDot);
    std::regex basePattern(R"(^Tile_([+\-]\d+)_([+\-]\d+)\.osgb$)");
    std::regex lvlPattern(R"(^Tile_([+\-]\d+)_([+\-]\d+)_L(\d{2})_([A-Za-z0-9]+)\.osgb$)");
    for(const QString& dn : dirNames){
        QDir td(base.filePath(dn));
        QFileInfoList list = td.entryInfoList(QStringList() << "Tile_*.osgb", QDir::Files);
        for(const QFileInfo& fi : list){
            std::string name = fi.fileName().toStdString(); std::smatch m;
            if(std::regex_match(name, m, basePattern)){
                int xi = parseSignedTriple(QString::fromStdString(m[1]));
                int yi = parseSignedTriple(QString::fromStdString(m[2]));
                auto& rec = out[{xi,yi}]; rec.x=xi; rec.y=yi; rec.baseFile = fi.filePath();
                ++st.files;
            } else if(std::regex_match(name, m, lvlPattern)){
                int xi = parseSignedTriple(QString::fromStdString(m[1]));
                int yi = parseSignedTriple(QString::fromStdString(m[2]));
                int L  = std::stoi(m[3]);
                auto& rec = out[{xi,yi}]; rec.x=xi; rec.y=yi; rec.levelFiles[L].push_back(fi.filePath());
                ++st.files;
            }
        }
    }
    st.tiles = (int)out.size();
    for(auto& kv : out) st.levels += (int)kv.second.levelFiles.size();
    return st;
}

struct QueryParams {
    int xMin=0, xMax=0; // 坐标范围
    int yMin=0, yMax=0;
    std::vector<int> levels; // 需要的层级; 空=只取 baseFile
    bool includeAllLevelFiles = true; // true: 该层级所有文件; false: 只取该层级一文件
    bool fallbackToBase = true;       // 指定层级不存在时是否回退到 baseFile
};

// 查询: 返回符合坐标范围 & 层级条件的所有文件全路径列表
inline std::vector<QString> queryFiles(const TileMap& map, const QueryParams& q){
    std::vector<QString> result; result.reserve(512);
    for(int xi=q.xMin; xi<=q.xMax; ++xi){
        for(int yi=q.yMin; yi<=q.yMax; ++yi){
            auto it = map.find({xi,yi}); if(it==map.end()) continue;
            const TileRecord& rec = it->second;
            if(q.levels.empty()){
                if(!rec.baseFile.isEmpty()) result.push_back(rec.baseFile);
                continue;
            }
            for(int L : q.levels){
                auto lf = rec.levelFiles.find(L);
                if(lf!=rec.levelFiles.end() && !lf->second.empty()){
                    if(q.includeAllLevelFiles){
                        result.insert(result.end(), lf->second.begin(), lf->second.end());
                    } else {
                        result.push_back(lf->second.front());
                    }
                } else if(q.fallbackToBase && !rec.baseFile.isEmpty()) {
                    result.push_back(rec.baseFile); // 回退
                }
            }
        }
    }
    return result;
}

} // namespace TileCatalog

#endif // TILE_CATALOG_H
