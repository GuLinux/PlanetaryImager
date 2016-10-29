## 0.6.2 2016-10-30
 - Fix ASI ROI validation, particularly with USB2 cameras
 - Fix ASI SDK version number
 - Add OpenGL support in image view, for better performances
 - Initial support for The Imaging Source (DMK) cameras (possibly Celestron too)
 - Fixed crash on camera disconnected right after changing a control
 - Added some simple error management and reporting dialog

## 0.6.1 2016-09-28
 - Histogram: allow to freely resize widget
 - Histogram: replaced "enable" checkbox with toggle logarithmic mode
 - Histogram: better resolution for plot image
 
## 0.6.0 2016-09-27
 - Improved colour camera support: display and save both bayer and debayered image, along with RGB formats
 - Optional debayer filter for display configurable in the main settings
 - Allow to choose recording duration based on frames number or seconds
 - Removed deprecated edge detection algorithm, using only OpenCV provided canny and sobel. This allows compilation in other platforms (for instance, arm)
 - ASI: Updated ASI SDK to v0.3.920, with great speed improvements for USB3 cameras, and fixing long exposures bug
 - QHY: use static libraries instead of installing dynamic libraries to the system
 - Improved threading for cameras: a lot of infrastructure refactoring to improve both performances and stability
 - Temperature polling in statusbar
 - Histogram: allow to toggle on/off and to configure interval to reduce CPU usage
 - Histogram: simplified draw to remove dependencies and solve plotting bugs
 - Lots of more configurable options
 - More reliable frame creation timestamp
 - Window size is now saved and restored during sessions
 - Notify user of controls accepted/rejected from camera, also disable "apply" button when no controls have pending changes
 - Fixes to QHY driver
 - Fixes to V4L2 driver
 
## 0.5.0 2016-08-21
 - Fix ASI controls issue
 - Fix UI settings value refresh
 - Support more control properties, such as readonly, auto
 - Added duration control widget for better editing exposure, etc

## 0.4.0 2016-08-12
 - Many stability fixes and improvements
 - Added ZWO ASI Support
 - Added ROI and bin (ZWO only)
 - Save camera settings in txt file
 - 16bit support
 - Histogram
 - Update QHY SDK, fixes many previous issues

## 0.3.0 2015-08-25
 - Removed Qt 5.5 dependency, now it compiles with Qt 5.4 too (found in much more distributions)
 - Edge detection for focus help
 - Support for color cameras, still untested but it might work
 - Added ser_extract_frames, command line utility to select single frames and ranges to a new file
 
## 0.2.1 2015-08-15
 - Minor update, with a .desktop launcher and icon

## 0.2.0 2015-08-15
 - Improved stability and speed
 - Enrich .SER file header with shoot information (observer, instrument, timestamp)

## 0.1.0 2015-08-11
 - Initial release: support for preview and live recording on monochrome cameras
