/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2016 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#ifndef WEATHER_SKYSIGHT_LAYERS_HPP
#define WEATHER_SKYSIGHT_LAYERS_HPP

#include "Util/tstring.hpp"
#include "Time/BrokenDateTime.hpp"
#include <map>

struct LegendColor {
  unsigned char Red;
  unsigned char Green;
  unsigned char Blue;
};

struct SkysightLayerDescriptor {
  const tstring id;
  const tstring name;
  const tstring desc;
  uint64_t last_update = 0;
  std::map<float, LegendColor> legend;
 // std::map<uint64_t, tstring> datafiles;

public:
  SkysightLayerDescriptor(tstring _id, tstring _name, tstring _desc) : id(_id), name(_name), desc(_desc) {}
  SkysightLayerDescriptor(const SkysightLayerDescriptor &descriptor) : id(descriptor.id), name(descriptor.name), desc(descriptor.desc), last_update(descriptor.last_update), legend(descriptor.legend) {}
};


struct SkysightStandbyLayer {
  SkysightLayerDescriptor *metric;
  uint64_t from = 0;
  uint64_t to = 0;
  uint64_t mtime = 0;
  bool updating = false;
  
public:
  SkysightStandbyLayer(SkysightLayerDescriptor *descriptor, uint64_t _from, uint64_t _to, uint64_t _mtime) : 
                                        metric(descriptor), from(_from), to(_to), mtime(_mtime) {}
  SkysightStandbyLayer(const SkysightStandbyLayer &layer) : metric(layer.metric), from(layer.from), 
to(layer.to), mtime(layer.mtime), updating(layer.updating) {}
};

struct SkysightDisplayedLayer {
  SkysightLayerDescriptor *descriptor;
  BrokenDateTime forecast_index;
  SkysightDisplayedLayer() {
    descriptor = nullptr;
  };
  SkysightDisplayedLayer(SkysightLayerDescriptor *descriptor, BrokenDateTime _fc_index) : descriptor(descriptor),
                                                                  forecast_index(_fc_index) {};
  void clear() {
    descriptor = nullptr;
  }
  bool operator==(const SkysightDisplayedLayer &layer) {
    return (*this == layer.descriptor->id.c_str());
  };

  bool operator==(const TCHAR *const id) {
    if(!descriptor || !id)
      return false;

    return (descriptor->id.compare(id) == 0);
  };

  bool operator==(const BrokenDateTime &t) {

    if(!forecast_index.IsPlausible())
      return false;

    return (forecast_index.ToUnixTimeUTC() == t.ToUnixTimeUTC());
  }

  // self > t?
  bool operator>(const BrokenDateTime &t) {
    if(!forecast_index.IsPlausible())
      return false;

    return (forecast_index.ToUnixTimeUTC() > t.ToUnixTimeUTC());
  }

  // self < t?
  bool operator<(const BrokenDateTime &t) {
    if(!forecast_index.IsPlausible())
      return false;

    return (forecast_index.ToUnixTimeUTC() < t.ToUnixTimeUTC());
  }
  
};





#endif
