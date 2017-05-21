#include "qhycontrol.h"
#include "qhyccdstruct.h"
#include "qhyexception.h"
#include "c++/stringbuilder.h"
#include "qhyccd.h"
#include <QMap>
#include "Qt/strings.h"
using namespace std;
using namespace std::placeholders;
using namespace GuLinux;

DPTR_IMPL(QHYControl) {
    int id;
    CONTROL_ID qid;
    qhyccd_handle *handle;
    QHYControl *q;
    bool isAvailable = false;
    bool readOnly = true;
    double min, max, step, value;
    static QMap<int, QString> control_names();
    string error_message(const QString &action) const;
};

QHYControl::QHYControl(int id, qhyccd_handle *handle) : dptr(id, static_cast<CONTROL_ID>(id), handle, this)
{
    try {
        QHY_CHECK << IsQHYCCDControlAvailable(handle, d->qid) << d->error_message("checking if control is available for %1");
        d->isAvailable = true;
        reload();
        QHY_CHECK << GetQHYCCDParamMinMaxStep(handle, d->qid, &d->min, &d->max, &d->step) << d->error_message("getting min/max/step for %1");
        d->readOnly = false;
    } catch(const QHYException &e) {
        qDebug() << e.what();
    }
}

void QHYControl::reload()
{
    d->value = GetQHYCCDParam(d->handle, d->qid); // TODO: verify how to check for errors
}

void QHYControl::setValue(double value)
{
    QHY_CHECK << SetQHYCCDParam(d->handle, d->qid, value);
}


Imager::Control QHYControl::control() const
{
auto control = Imager::Control{ static_cast<qlonglong>(Imager::Control::Number), name() }
    .set_id(d->id)
    .set_range(d->min, d->max, d->step)
    .set_value(d->value)
    .set_default_value(0)
    .set_decimals(2)
    ;
    if(d->qid == CONTROL_EXPOSURE) {
        control.is_duration = true;
        control.duration_unit = 1us;
        control.is_exposure = true;
    }
    control.readonly = d->readOnly;
    return control;
}



string QHYControl::Private::error_message(const QString &action) const {
    return action.arg("%1 [%2]"_q % control_names()[id] % id).toStdString();
}

QHYControl::~QHYControl()
{
}

int QHYControl::id() const
{
    return d->id;
}

QString QHYControl::name() const
{
    return Private::control_names()[id()];
}

bool QHYControl::isUIControl() const
{
    static list<int> ui_control_ids{
    CONTROL_BRIGHTNESS, //= 0, //!< image brightness
    CONTROL_CONTRAST,       //!< image contrast
    CONTROL_WBR,            //!< red of white balance
    CONTROL_WBB,            //!< blue of white balance
    CONTROL_WBG,            //!< the green of white balance
    CONTROL_GAMMA,          //!< screen gamma
    CONTROL_GAIN,           //!< camera gain
    CONTROL_OFFSET,         //!< camera offset
    CONTROL_EXPOSURE,       //!< expose time (us)
    CONTROL_SPEED,          //!< transfer speed
    CONTROL_TRANSFERBIT,    //!< image depth bits
    CONTROL_CHANNELS,       //!< image channels
    CONTROL_USBTRAFFIC,     //!< hblank
    CONTROL_ROWNOISERE,     //!< row denoise
    CONTROL_CURTEMP,        //!< current cmos or ccd temprature
    CONTROL_CURPWM,         //!< current cool pwm
    CONTROL_MANULPWM,       //!< set the cool pwm
    CONTROL_CFWPORT,        //!< control camera color filter wheel port
    CONTROL_COOLER,         //!< check if camera has cooler
    CONTROL_ST4PORT,        //!< check if camera has st4port
    CAM_COLOR,
    CAM_BIN1X1MODE,         //!< check if camera has bin1x1 mode
    CAM_BIN2X2MODE,         //!< check if camera has bin2x2 mode
    CAM_BIN3X3MODE,         //!< check if camera has bin3x3 mode
    CAM_BIN4X4MODE,         //!< check if camera has bin4x4 mode
    CAM_MECHANICALSHUTTER,                   //!< mechanical shutter
    CAM_TRIGER_INTERFACE,                    //!< triger
    CAM_TECOVERPROTECT_INTERFACE,            //!< tec overprotect
    CAM_SINGNALCLAMP_INTERFACE,              //!< singnal clamp
    CAM_FINETONE_INTERFACE,                  //!< fine tone
    CAM_SHUTTERMOTORHEATING_INTERFACE,       //!< shutter motor heating
    CAM_CALIBRATEFPN_INTERFACE,              //!< calibrated frame
    CAM_CHIPTEMPERATURESENSOR_INTERFACE,     //!< chip temperaure sensor
    CAM_USBREADOUTSLOWEST_INTERFACE,         //!< usb readout slowest

    CAM_8BITS,                               //!< 8bit depth
    CAM_16BITS,                              //!< 16bit depth
    CAM_GPS,                                 //!< check if camera has gps

    CAM_IGNOREOVERSCAN_INTERFACE,            //!< ignore overscan area

    QHYCCD_3A_AUTOBALANCE,
    QHYCCD_3A_AUTOEXPOSURE,
    QHYCCD_3A_AUTOFOCUS,
    CONTROL_AMPV,                            //!< ccd or cmos ampv
    CONTROL_VCAM                             //!< Virtual Camera on off
    };
    return find(begin(ui_control_ids), end(ui_control_ids), d->id) != end(ui_control_ids);
}

bool QHYControl::available() const
{
    return d->isAvailable;
}

// sync this with qhyccdstruct.h
// regex: (^\s*)(\w+)(.*)
// replace: \1{ \2, "" }\3
QMap<int, QString> QHYControl::Private::control_names() {
    return {
        { CONTROL_BRIGHTNESS, "Brightness" }, //= 0, //!< image brightness
        { CONTROL_CONTRAST, "Contrast" },       //!< image contrast
        { CONTROL_WBR, "R white balance" },            //!< red of white balance
        { CONTROL_WBB, "B white balance" },            //!< blue of white balance
        { CONTROL_WBG, "G white balance" },            //!< the green of white balance
        { CONTROL_GAMMA, "Gamma" },          //!< screen gamma
        { CONTROL_GAIN, "Gain" },           //!< camera gain
        { CONTROL_OFFSET, "Offset" },         //!< camera offset
        { CONTROL_EXPOSURE, "Exposure" },       //!< expose time (us)
        { CONTROL_SPEED, "Transfer speed" },          //!< transfer speed
        { CONTROL_TRANSFERBIT, "Depth" },    //!< image depth bits
        { CONTROL_CHANNELS, "Channels" },       //!< image channels
        { CONTROL_USBTRAFFIC, "USB Traffic" },     //!< hblank
        { CONTROL_ROWNOISERE, "Row denoise" },     //!< row denoise
        { CONTROL_CURTEMP, "Current Temp" },        //!< current cmos or ccd temprature
        { CONTROL_CURPWM, "Current PWM" },         //!< current cool pwm
        { CONTROL_MANULPWM, "Set PWD" },       //!< set the cool pwm
        { CONTROL_CFWPORT, "Camera filter wheel port" },        //!< control camera color filter wheel port
        { CONTROL_COOLER, "Has cooler" },         //!< check if camera has cooler
        { CONTROL_ST4PORT, "Has ST4 port" },        //!< check if camera has st4port
        { CAM_COLOR, "Color" },
        { CAM_BIN1X1MODE, "Has bin1" },         //!< check if camera has bin1x1 mode
        { CAM_BIN2X2MODE, "Has bin2" },         //!< check if camera has bin2x2 mode
        { CAM_BIN3X3MODE, "Has bin3" },         //!< check if camera has bin3x3 mode
        { CAM_BIN4X4MODE, "Has bin4" },         //!< check if camera has bin4x4 mode
        { CAM_MECHANICALSHUTTER, "Mechanical shutter" },                   //!< mechanical shutter
        { CAM_TRIGER_INTERFACE, "Trigger interface" },                    //!< triger
        { CAM_TECOVERPROTECT_INTERFACE, "Tec overprotect" },            //!< tec overprotect
        { CAM_SINGNALCLAMP_INTERFACE, "Signal clamp" },              //!< singnal clamp
        { CAM_FINETONE_INTERFACE, "Fine tone" },                  //!< fine tone
        { CAM_SHUTTERMOTORHEATING_INTERFACE, "Shutter motor heating" },       //!< shutter motor heating
        { CAM_CALIBRATEFPN_INTERFACE, "Calibrate Frame" },              //!< calibrated frame
        { CAM_CHIPTEMPERATURESENSOR_INTERFACE, "Chip temperature sensor" },     //!< chip temperaure sensor
        { CAM_USBREADOUTSLOWEST_INTERFACE, "USB Readout slowest" },         //!< usb readout slowest

        { CAM_8BITS, "8bits" },                               //!< 8bit depth
        { CAM_16BITS, "16bits" },                              //!< 16bit depth
        { CAM_GPS, "Has GPS" },                                 //!< check if camera has gps

        { CAM_IGNOREOVERSCAN_INTERFACE, "Ignore overscan" },            //!< ignore overscan area

        { QHYCCD_3A_AUTOBALANCE, "Autobalance" },
        { QHYCCD_3A_AUTOEXPOSURE, "Autoexposure" },
        { QHYCCD_3A_AUTOFOCUS, "Autofocus" },
        { CONTROL_AMPV, "Ampv" },                            //!< ccd or cmos ampv
        { CONTROL_VCAM, "VCam" }                             //!< Virtual Camera on off
    };
}

//#include <QFile>
//#include <QJsonDocument>
QList<QHYControl::ptr> QHYControl::availableControls(qhyccd_handle *handle)
{
    auto keys = Private::control_names().keys();
    QList<QHYControl::ptr> controls;
    transform(begin(keys), end(keys), back_inserter(controls), [=](auto id){ return make_shared<QHYControl>(id, handle);});
    controls.erase(remove_if(begin(controls), end(controls), [](const auto &c) { return !c->available(); }), end(controls));

//    QFile json_file("/tmp/qhycontrols.json");
//    json_file.open(QIODevice::WriteOnly);
//    json_file.write("{\n");
//    QVariantMap controls_json;
//    QString sep;
//    for(auto control: controls) {
//        json_file.write(QString(R"(%1    %2: { "name": "%3", "readonly": %4, "min": %5, "max": %6, "step": %7, "value": %8})")
//                        .arg(sep)
//                .arg(control->id(), 2)
//                .arg(control->name(), 15)
//                .arg(control->d->readOnly, 1)
//                .arg(control->d->min, 11)
//                .arg(control->d->max, 11)
//                .arg(control->d->step, 11)
//                .arg(control->d->value, 20, 'f')
//                        .toLatin1()
//        );
//        sep = ",\n";
//    }
//    json_file.write("\n}\n");
    return controls;
}
