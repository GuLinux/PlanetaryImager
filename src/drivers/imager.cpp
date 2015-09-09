#include "imager.h"

QDebug operator<<(QDebug dbg, const Imager::Chip& chip)
{
  dbg.nospace() << "{ size: " << chip.width << "x" << chip.height << ", pixels size: " << chip.pixelwidth << "x" << chip.pixelheight <<
    ", image size: " << chip.xres << "x" << chip.yres << "@" << chip.bpp << "bpp }";
  return dbg.space();
}

QDebug operator<<(QDebug dbg, const Imager::Setting& setting)
{
  static QMap<Imager::Setting::Type, QString> types_map {
    {Imager::Setting::Number, "Number"},
    {Imager::Setting::String, "String"},
    {Imager::Setting::Combo, "Combo"},
    {Imager::Setting::Bool, "Bool"},
  };
  dbg.nospace() << "{ name: " << setting.name << ", min: " << setting.min << ", max: " << setting.max << ", step: " << setting.step << ", value: " << setting.value 
  << ", type: " << types_map[setting.type] << ", choices: " << setting.choices << ", default: " << setting.defaut_value << " }";
  return dbg.space();
}

Imager::Setting::operator bool() const
{
    return ! name.isEmpty();
}
