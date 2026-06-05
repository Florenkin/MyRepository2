#pragma once

#ifndef GLOBALDATA_H
#define GLOBALDATA_H

#if defined(_MSC_VER)
#pragma warning(push, 0)  // ignore all warnings
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"  // ignore all warnings
#endif

#include <QObject>
#include <QRectF>
#include <QList>
#include <QColor>
#include <QDataStream>
#include <QDebug>
#include <QPixmap>
#include <QGraphicsItemGroup>

#if defined(_MSC_VER)
#pragma warning(pop)  // restore warnings
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop  // restore warnings
#endif

#ifdef _WIN32
#if defined(GLOBAL_DATA_LIBRARY)
#define GLOBAL_DATA_EXPORT __declspec(dllexport)
#else
#define GLOBAL_DATA_EXPORT __declspec(dllimport)
#endif
#else
#define GLOBAL_DATA_EXPORT
#endif

/**
 *全局数据结构，用于各个子模块的之间的数据交互
 */
namespace GlobalData
{
    //scale factor between inch and pixel, 1 inch = 25.4*100 pixels; 1 mm = 100 pixels;
    const double ScaleFactor = 25.4 / 0.01; 	//one pixel is 0.01mm

    class Rect3D : public QRectF
    {
    public:
        Rect3D(){}

        explicit Rect3D(double width, double hight, double hight3D){

            this->setWidth(width);
            this->setHeight(hight);
            this->setHeight3d(hight3D);
        }

        Rect3D(const GlobalData::Rect3D& copy){

            this->setRect(copy.x(), copy.y(), copy.width(), copy.height());
            this->_height3d = copy._height3d;
        }


        Rect3D& operator=(const GlobalData::Rect3D& copy){

            this->setRect(copy.x(), copy.y(), copy.width(), copy.height());
            this->_height3d = copy._height3d;

            return *this;
        }

        ~Rect3D(){}

        Rect3D scale(double scaleFactor)
        {
            auto width    = this->width()*scaleFactor;
            auto height   = this->height()*scaleFactor;
            auto height3D = this->_height3d*scaleFactor;

            return Rect3D(width, height, height3D);
        }

        QRectF rect2D(){
            return QRectF(this->x(), this->y(), this->width(), this->height());
        }

        double height3D() const
        {
            return _height3d;
        }

        void setRect2D(QRectF rect)
        {
            this->setRect(rect.x(), rect.y(), rect.width(), rect.height());
        }

        void setHeight3d(double height)
        {
            this->_height3d = height;
        }

        Rect3D operator*(const double scale) const{
            auto width    = this->width()*scale;
            auto height   = this->height()*scale;
            auto height3D = this->_height3d*scale;

            return Rect3D(width, height, height3D);
        }

        /*
        Rect3D& operator=(const Rect3D &other) const{
            this->setRect(other.x(), other.y(), other.width(), other.height());
            this->_height3d = other._height3d;
        }
        */

        friend QDebug operator<< (QDebug d, const Rect3D &data) {
                d << data.x() << data.y() << data.width() << data.height() << data.height3D() << data.center();
                return d;
            }

    private:
        double _height3d;
    };

    enum PackageDataType{
        EM_TYPE,
        EM_INSTANCE         //a specify component or libaray
    };

    //package data 封装数据
    struct PackageData{

        PackageData(){}

        PackageData(PackageData const& data){

            this->typeName      = data.typeName;
            this->instanceName  = data.instanceName;

            this->alias         = data.alias;
            this->uuid          = data.uuid;

            this->body       = data.body;
            this->searchArea = data.searchArea;
            this->leads      = data.leads;
            this->type       = data.type;

            this->bodyColor = data.bodyColor;
            this->searchAreaColor = data.searchAreaColor;
            this->leadColor = data.leadColor;
            this->snapShoot = data.snapShoot;
        }

        //id name
        QString         typeName;           //"C0603"
        QString         instanceName;       //"R16"

        QStringList     alias;
        QString         uuid;

        PackageDataType type{EM_TYPE};

        Rect3D          body;
        Rect3D          searchArea;
        QList<GlobalData::Rect3D>   leads;

        QColor          bodyColor;
        QColor          searchAreaColor;
        QColor          leadColor;

        QPixmap         snapShoot;

        friend QDebug operator << (QDebug debug, const GlobalData::PackageData& data) {

        QString bodyStr = QString("width: %1, height:%2, height3d: %3, centerX: %4, centerY: %5")
                .arg(data.body.width())
                .arg(data.body.height())
                .arg(data.body.height3D())
                .arg(data.body.center().x())
                .arg(data.body.center().y());

        QString searchAreaStr = QString("width: %1, height:%2, height3d: %3, centerX: %4, centerY: %5")
                .arg(data.searchArea.width())
                .arg(data.searchArea.height())
                .arg(data.searchArea.height3D())
                .arg(data.searchArea.center().x())
                .arg(data.searchArea.center().y());

        QString info = QString("typeName: %1 instanceName: %2 alias: %3  uuid: %4  body: %5  searchArea: %6")
                .arg(data.typeName)
                .arg(data.instanceName)
                .arg(data.alias.join("-"))
                .arg(data.uuid)
                .arg(bodyStr)
                .arg(searchAreaStr);

            debug << info;
            return debug;
        }

        friend bool operator<(const PackageData& p1, const PackageData& p2){

            bool ret = false;
            if(p1.instanceName.length() != p2.instanceName.length()){
                ret = p1.instanceName.length() > p2.instanceName.length() ? true : false;
            }
            else{
                auto length = p1.instanceName.length();

                for(int i = 0; i<length; ++i){
                    auto char_p1 = p1.instanceName.at(i);
                    auto char_p2 = p2.instanceName.at(i);

                    if(char_p1 != char_p2){
                        ret = char_p1 > char_p2 ? true : false;
                        break;
                    }
                }
            }
            return ret;
        }
    };

    GLOBAL_DATA_EXPORT PackageData TransformPackage(const PackageData& package, const QPointF& center);
    GLOBAL_DATA_EXPORT QRectF PackageBoundingRect(const PackageData& package);
    GLOBAL_DATA_EXPORT void MakePackageSnapshot(PackageData& package);

    struct ROIData
    {
        ROIData() {}
        ROIData(const ROIData& data){

            this->rect  = data.rect;
            this->id    = data.id;
            this->uuid  = data.uuid;
            this->color = data.color;
			this->name  = data.name;
        }

        QRectF  rect;
        QString id;
        QString uuid;
		QString name;
        QColor  color;
    };

    const double InchToMM = 25.4;

    enum InspectMode{
        Em_InspectAll,
        Em_InspectCurrentPackage,
        Em_InspectCurrentRoi
    };

    struct InspectResultData{
        QString packageUuid;
        QString packageName;

        QString roiUuid;
        QString roiName;
        
        QString algorithmUuid;
        bool    result;
    };

    struct AlgorithmData
    {
        AlgorithmData(){}

        QString     name;
        uint        id;             //can not repeat
        QString     uuid;           //unique id
        QString     desc;
        QString     version;
        QString     path;

        friend QDebug operator << (QDebug debug, const GlobalData::AlgorithmData& data) {

        QString info = QString("name: %1  id: %2  uuid: %3  desc: %4  version: %5  path: %6")
                .arg(data.name)
                .arg(data.id)
                .arg(data.uuid)
                .arg(data.desc)
                .arg(data.version)
                .arg(data.path);

            debug << info;
            return debug;
        }
    };

    struct ConfigResultData{

        PackageData          packageData;
        QList<PackageData>   roiDataList;
        QList<AlgorithmData> selectedAlgoDataList;

        void Print(){
            qDebug()<<packageData<<"--"<<roiDataList<<"--"<<selectedAlgoDataList;
        }
    };

	enum TreeWidgetItemType
	{
		em_package,
		em_roi
	};

};

Q_DECLARE_METATYPE(GlobalData::Rect3D);
Q_DECLARE_METATYPE(GlobalData::ROIData);
Q_DECLARE_METATYPE(GlobalData::PackageData);
Q_DECLARE_METATYPE(GlobalData::AlgorithmData);
Q_DECLARE_METATYPE(GlobalData::ConfigResultData);

GLOBAL_DATA_EXPORT QDataStream& operator << (QDataStream& out, const GlobalData::Rect3D& data);
GLOBAL_DATA_EXPORT QDataStream& operator >> (QDataStream& in,  GlobalData::Rect3D& data);

GLOBAL_DATA_EXPORT QDataStream& operator << (QDataStream& out, const GlobalData::PackageData& data);
GLOBAL_DATA_EXPORT QDataStream& operator >> (QDataStream& in,  GlobalData::PackageData& data);

GLOBAL_DATA_EXPORT QDataStream& operator << (QDataStream& out, const GlobalData::AlgorithmData& data);
GLOBAL_DATA_EXPORT QDataStream& operator >> (QDataStream& in,  GlobalData::AlgorithmData& data);

GLOBAL_DATA_EXPORT QDataStream& operator << (QDataStream& out, const GlobalData::ConfigResultData& data);
GLOBAL_DATA_EXPORT QDataStream& operator >> (QDataStream& in,  GlobalData::ConfigResultData& data);

//qRegisterMetaTypeStreamOperators<GlobalData::Rect3D>("GlobalData::Rect3D");
//qRegisterMetaTypeStreamOperators<GlobalData::PackageData>("GlobalData::PackageData");

#endif // GLOBALDATA_H
