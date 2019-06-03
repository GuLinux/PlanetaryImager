using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using ZWOptical.ASISDK;

namespace ZWOptical.ASISDK.ObjectModel
{
    public static class ASICameras
    {

        private static readonly Camera[] _cameras = new Camera[16]; //16

        public static int Count
        {
            get { return ASICameraDll.GetNumOfConnectedCameras(); }
        }

        public static Camera GetCameraByIndex(int cameraIndex)
        {  
            if (cameraIndex >= Count || cameraIndex < 0)
                throw new IndexOutOfRangeException();

            ASICameraDll.ASI_CAMERA_INFO infoTemp = ASICameraDll.GetCameraProperties(cameraIndex);
            int cameraId = infoTemp.CameraID;
            return _cameras[cameraId] ?? (_cameras[cameraId] = new Camera(cameraId));
        }
    
    }
    public enum ASI_STATUS
    {
        CLOSED = 0,
        OPENED,
        EXPOSURING
    }
    public class Camera
    {
        private readonly int _cameraId;
        private string _cachedName;
        private List<CameraControl> _controls;
        private ASICameraDll.ASI_CAMERA_INFO? _info;
        private ASI_STATUS _status;
        public Camera(int cameraId)
        {
            _cameraId = cameraId;
        }

        private ASICameraDll.ASI_CAMERA_INFO Info
        {
            // info is cached only while camera is open
            get { return _info ?? ASICameraDll.GetCameraProperties(_cameraId); }
        }

        public ASI_STATUS status
        {
            get { return _status; }
        }

        public string Name
        {
            get { return Info.Name; } 
        }

        public bool IsColor
        {
            get { return Info.IsColorCam != ASICameraDll.ASI_BOOL.ASI_FALSE; }
        }

        public bool HasST4
        {
            get { return Info.ST4Port != ASICameraDll.ASI_BOOL.ASI_FALSE; }
        }

        public bool HasShutter
        {
            get { return Info.MechanicalShutter != ASICameraDll.ASI_BOOL.ASI_FALSE; }
        }

        public bool HasCooler
        {
            get { return Info.IsCoolerCam != ASICameraDll.ASI_BOOL.ASI_FALSE; }
        }

        public bool IsUSB3
        {
            get { return Info.IsUSB3Host != ASICameraDll.ASI_BOOL.ASI_FALSE; }
        }

        public int CameraId
        {
            get { return _cameraId; }
        }

        public ASICameraDll.ASI_BAYER_PATTERN BayerPattern
        {
            get { return Info.BayerPattern; }
        }

        public Size Resolution
        {
            get
            {
                var info = Info;
                return new Size(info.MaxWidth, info.MaxHeight);
            }
        }

        public double PixelSize
        {
            get { return Info.PixelSize; }
        }

        public List<int> SupportedBinFactors
        {
            get { return Info.SupportedBins.TakeWhile(x => x != 0).ToList(); }
        }

        public List<ASICameraDll.ASI_IMG_TYPE> SupportedImageTypes
        {
            get {
                
                return Info.SupportedVideoFormat.TakeWhile(x => x != ASICameraDll.ASI_IMG_TYPE.ASI_IMG_END).ToList(); }
        }

        public ASICameraDll.ExposureStatus ExposureStatus
        {
            get { 
                 
                ASICameraDll.ExposureStatus status = ASICameraDll.GetExposureStatus(_cameraId);
                if (status != ASICameraDll.ExposureStatus.ExpWorking)
                    _status = ASI_STATUS.OPENED;
                return status;
            }
        }

        public void OpenCamera()
        {
            ASICameraDll.OpenCamera(_cameraId);
            _info = ASICameraDll.GetCameraProperties(_cameraId);
            ASICameraDll.InitCamera(_cameraId);
            _status = ASI_STATUS.OPENED;
        }

        public void CloseCamera()
        {
            _info = null;
            _controls = null;
            ASICameraDll.CloseCamera(_cameraId);
            _status = ASI_STATUS.CLOSED;
        }

        public List<CameraControl> Controls
        {
            get
            {
                if (_controls == null || _cachedName != Name)
                {
                    _cachedName = Name;
                    int cc = ASICameraDll.GetNumOfControls(_cameraId);
                    _controls = new List<CameraControl>();
                    for (int i = 0; i < cc; i++)
                    {
                        _controls.Add(new CameraControl(_cameraId, i));
                    }
                }

                return _controls;
            }
        }

        public Point StartPos
        {
            get { return ASICameraDll.GetStartPos(_cameraId); }
            set { ASICameraDll.SetStartPos(_cameraId, value); }
        }

        public CaptureAreaInfo CaptureAreaInfo
        {
            get
            {
                int bin;
                ASICameraDll.ASI_IMG_TYPE imageType;
                var res = ASICameraDll.GetROIFormat(_cameraId, out bin, out imageType);
                return new CaptureAreaInfo(res, bin, imageType);
            }
            set
            {
                ASICameraDll.SetROIFormat(_cameraId, value.Size, value.Binning, value.ImageType);
            }
        }

        public int DroppedFrames
        {
            get { return ASICameraDll.GetDroppedFrames(_cameraId); }
        }

        public bool EnableDarkSubtract(string darkImageFilePath)
        {
            return ASICameraDll.EnableDarkSubtract(_cameraId, darkImageFilePath);
        }

        public void DisableDarkSubtract()
        {
            ASICameraDll.DisableDarkSubtract(_cameraId);
        }

        public void StartVideoCapture()
        {
            ASICameraDll.StartVideoCapture(_cameraId);
            _status = ASI_STATUS.EXPOSURING;
        }

        public void StopVideoCapture()
        {
            ASICameraDll.StopVideoCapture(_cameraId);
            _status = ASI_STATUS.OPENED;
        }

        public bool GetVideoData(IntPtr buffer, int bufferSize, int waitMs)
        {
            return ASICameraDll.GetVideoData(_cameraId, buffer, bufferSize, waitMs);
        }

        public void PulseGuideOn(ASICameraDll.ASI_GUIDE_DIRECTION direction)
        {
            ASICameraDll.PulseGuideOn(_cameraId, direction);
        }

        public void PulseGuideOff(ASICameraDll.ASI_GUIDE_DIRECTION direction)
        {
            ASICameraDll.PulseGuideOff(_cameraId, direction);
        }

        public void StartExposure(int exposureMs, bool isDark)
        {
            ASICameraDll.StartExposure(_cameraId, exposureMs, isDark);
            _status = ASI_STATUS.EXPOSURING;
        }

        public void StopExposure()
        {
            ASICameraDll.StopExposure(_cameraId);
        }

        public bool GetExposureData(IntPtr buffer, int bufferSize)
        {
            return ASICameraDll.GetDataAfterExp(_cameraId, buffer, bufferSize);
        }

        public CameraControl GetControl(ASICameraDll.ASI_CONTROL_TYPE controlType)
        {
            return Controls.FirstOrDefault(x => x.ControlType == controlType);
        }
    }

    public class CaptureAreaInfo
    {
        public Size Size { get; set; }
        public int Binning { get; set; }
        public ASICameraDll.ASI_IMG_TYPE ImageType { get; set; }

        public CaptureAreaInfo(Size size, int binning, ASICameraDll.ASI_IMG_TYPE imageType)
        {
            Size = size;
            Binning = binning;
            ImageType = imageType;
        }
    }

    public class CameraControl
    {
        private readonly int _cameraId;
        private ASICameraDll.ASI_CONTROL_CAPS _props;
        private bool _auto;

        public CameraControl(int cameraId, int controlIndex)
        {
            _cameraId = cameraId;

            _props = ASICameraDll.GetControlCaps(_cameraId, controlIndex);
            _auto = GetAutoSetting();
        }

        public string Name { get { return _props.Name; } }
        public string Description { get { return _props.Description; } }
        public int MinValue { get { return _props.MinValue; } }
        public int MaxValue { get { return _props.MaxValue; } }
        public int DefaultValue { get { return _props.DefaultValue; } }
        public ASICameraDll.ASI_CONTROL_TYPE ControlType { get { return _props.ControlType; } }
        public bool IsAutoAvailable { get { return _props.IsAutoSupported != ASICameraDll.ASI_BOOL.ASI_FALSE; } }
        public bool Writeable { get { return _props.IsWritable != ASICameraDll.ASI_BOOL.ASI_FALSE; } }

        public int Value
        {
            get
            {
                bool isAuto;
                return ASICameraDll.GetControlValue(_cameraId, _props.ControlType, out isAuto);
            }
            set
            {
                ASICameraDll.SetControlValue(_cameraId, _props.ControlType, value, IsAuto);                
            }
        }

        public bool IsAuto
        {
            get 
            {
                return _auto;
            }
            set
            {
                _auto = value;
                ASICameraDll.SetControlValue(_cameraId, _props.ControlType, Value, value);
            }
        }

        private bool GetAutoSetting()
        {
            bool isAuto;
            ASICameraDll.GetControlValue(_cameraId, _props.ControlType, out isAuto);
            return isAuto;
        }
    }
}
