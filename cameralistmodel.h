#include <QAbstractListModel>
#include <QCameraInfo>

Q_DECLARE_METATYPE(QCameraInfo) // put this in a header file
// Custom model to wrap QList<Camera>
class CameraListModel : public QAbstractListModel {
public:
    // Optionally implement roleNames method for custom roles
    CameraListModel(): QAbstractListModel(nullptr), m_cameras(){};
    CameraListModel(QObject *parent): QAbstractListModel(parent), m_cameras(){};

    CameraListModel(const QList<QCameraInfo>& cameras, QObject* parent = nullptr)
        : QAbstractListModel(parent), m_cameras(cameras) {
        qRegisterMetaType<QCameraInfo>();
    }

    int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        Q_UNUSED(parent);
        return m_cameras.size();
    }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
        if (!index.isValid() || index.row() >= m_cameras.size())
            return QVariant();

        const QCameraInfo& camera = m_cameras.at(index.row());
        if (role == Qt::DisplayRole)
            return camera.description(); // Adjust as needed

        return QVariant();
    }

    QCameraInfo cameraInfo(int index) const {
        if (index < 0 || index >= m_cameras.size())
            return QCameraInfo();
        return m_cameras.at(index);
    }



private:
    QList<QCameraInfo> m_cameras;
};

