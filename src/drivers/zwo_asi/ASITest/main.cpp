/*
 * Copyright (C) 2016  Marco Gulino <marco@gulinux.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <iostream>
#include <vector>
#include <sstream>
#include <atomic>
#include <thread>
#include <chrono>
#include <memory>
#include "ASICamera2.h"

using namespace std;
using namespace std::chrono_literals;

ostream &operator<<(ostream &o, const ASI_CAMERA_INFO &ci) {
  o << "{ id=" << ci.CameraID << ", name=" << ci.Name << ", maxWidth: " << ci.MaxWidth
    << ", maxHeight: " << ci.MaxHeight  << ", color: " << ci.IsColorCam
    << ", bayer pattern: " << ci.BayerPattern << ", pixsize: " << ci.PixelSize
    << ", usb3: " << ci.IsUSB3Camera << ", usb3 host: " << ci.IsUSB3Host << " }";
  return o;
}


ostream &operator<<(ostream &o, const ASI_CONTROL_CAPS &c) {
  o << "{ " << c.Description << " (" << c.Name << "): min=" << c.MinValue << ", max="
    << c.MaxValue << ", default= " << c.DefaultValue << ", auto=" << c.IsAutoSupported
    << ", writable=" << c.IsWritable << " }";
  return o;
}

void check_result(int result, const std::string &operation) {
  if(result != ASI_SUCCESS) {
    stringstream s;
    s << "Error executing " << operation << ": " << result;
    throw runtime_error(s.str());
  }
}

#define CHECK_RESULT(operation) check_result(operation, #operation)

struct ASIControl {
    typedef std::vector<ASIControl> vector;
    int index;
    int camera_id;
    ASI_CONTROL_CAPS control;
    long value;
    bool is_auto;
    const ASI_CONTROL_CAPS *operator->() const { return &control; }
    ASIControl &read();
    ASIControl &set(long value, bool set_auto);
    static ASIControl create(int camera_id, int index);
};



ASIControl &ASIControl::read()
{
  ASI_BOOL is_auto;
  CHECK_RESULT( ASIGetControlValue(camera_id, control.ControlType, &value, &is_auto) );
  this->is_auto = is_auto;
  return *this;
}

ASIControl &ASIControl::set(long value, bool set_auto)
{
  CHECK_RESULT(ASISetControlValue(camera_id, control.ControlType, value, set_auto ? ASI_TRUE : ASI_FALSE));
  this->value = value;
  this->is_auto = set_auto;
  return *this;
}

ASIControl ASIControl::create(int camera_id, int index)
{
  ASI_CONTROL_CAPS caps;
  CHECK_RESULT(ASIGetControlCaps(camera_id, index, &caps));
  return ASIControl{index, camera_id, caps}.read();
}


ostream &operator<<(ostream &o, ASIControl c) {
  c.read();
  o << "Control " << c.index << ": " << c.control << ", value=";
  if(c.is_auto)
    o << "AUTO";
  else
    o << c.value;
  return o;
}

class ASICamera {
public:
  ASICamera(const ASI_CAMERA_INFO &info);
  ~ASICamera();
  int id() const { return info.CameraID; }
  void print_controls(ostream &out) const;
  ASIControl::vector controls() const { return _controls; }
  size_t buffer_size() const { return info.MaxHeight * info.MaxWidth; }
private:
  ASI_CAMERA_INFO info;
  ASIControl::vector _controls;
};

ASICamera::ASICamera(const ASI_CAMERA_INFO &info) : info{info}
{
  CHECK_RESULT( ASIOpenCamera(id()) );
  CHECK_RESULT( ASIInitCamera(id()) );
  int controls_num;
  CHECK_RESULT(ASIGetNumOfControls(id(),  &controls_num) );
  _controls.resize(controls_num);
  int index = 0;
  for(auto &control: _controls) {
    control = ASIControl::create(id(), index++);
  }
  CHECK_RESULT(ASISetROIFormat(id(), info.MaxWidth, info.MaxHeight, 1, ASI_IMG_RAW8));
  CHECK_RESULT(ASISetStartPos(id(), 0, 0));
}

ASICamera::~ASICamera()
{
  ASICloseCamera(id());
}

void ASICamera::print_controls(ostream &out) const
{
  for(auto control: _controls) {
    out << control << endl;
  }
}

void mainControl()
{
    int result = ASIGetNumOfConnectedCameras();
    if(result <= 0) {
      throw runtime_error("No cameras found");
    }
    cout << "Found " << result << " cameras: " << endl;
    vector<ASI_CAMERA_INFO> camera_infos(result);
    int index = 0;
    for(auto &info: camera_infos) {
      result = ASIGetCameraProperty(&info, index);
      cout << index++ << ": " << info << endl;
    }
    cout << "Index of camera to use? ";
    cin >> index;

    auto camera = make_shared<ASICamera>(camera_infos[index]);

    auto set_value = [&](int index) {
      cout << camera->controls()[index] << endl;
      cout << "Enter value (a for auto): ";
      string s;
      cin >> s;
      stringstream ss(s);
      long v;
      ss >> v;
      camera->controls()[index].set(v, s == "a");
      camera->print_controls(cout);
    };

    atomic_bool do_capture;
    auto start_capture = [&] {
      do_capture = true;
      CHECK_RESULT(ASIStartVideoCapture(camera->id() ));
      cerr << "Video capture started\n";
      vector<uint8_t> buffer(camera->buffer_size() );
      auto start = chrono::steady_clock::now();
      size_t frames = 0;
      while(do_capture) {
        CHECK_RESULT(ASIGetVideoData(camera->id(), buffer.data(), buffer.size(), -1));
        frames++;
        auto now = chrono::steady_clock::now();
        chrono::duration<double> elapsed = now - start;
        if (elapsed > 2s) {
          cerr << "Captured: " << frames << ", in " << elapsed.count() << "s: FPS "
               << static_cast<double>(frames) / elapsed.count() << endl;
          start = now;
          frames = 0;
        }
      }
    };

    camera->print_controls(cout);

    shared_ptr<thread> capture_thread;
    string s;
    while(true) {
      cout << "Enter action (c: capture, s: stop capture, q: quit, 0-" << camera->controls().size()-1 << ": change setting, p: print controls) ";
      cin >> s;
      if(s == "c" && !do_capture) {
        capture_thread = make_shared<thread>(start_capture);
        capture_thread->detach();
      }
      if(s == "s")
        do_capture = false;
      if(s == "q")
        return;
      if(s == "p")
        camera->print_controls(cout);
      stringstream ss(s);
      ss >> index;
      if(index > -1 && index < camera->controls().size())
        set_value(index);
    }
}

int main(int argc, char *argv[])
{
  try {
        mainControl();
    } catch(std::exception &e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

  return 0;
}

