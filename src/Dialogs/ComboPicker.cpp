/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2013 The XCSoar Project
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

#include "Dialogs/ComboPicker.hpp"
#include "Dialogs/ListPicker.hpp"
#include "Form/List.hpp"
#include "Form/Edit.hpp"
#include "Form/DataField/Base.hpp"
#include "Form/DataField/ComboList.hpp"
#include "Screen/Canvas.hpp"
#include "Screen/Layout.hpp"
#include "UIGlobals.hpp"
#include "Look/DialogLook.hpp"

#include <assert.h>

static WndProperty *wComboPopupWndProperty;
static DataField *ComboPopupDataField;
static const ComboList *ComboListPopup;

class ComboPickerSupport : public ListItemRenderer {
  const ComboList &combo_list;
  const UPixelScalar padding;

public:
  ComboPickerSupport(const ComboList &_combo_list,
                     const UPixelScalar _padding)
    :combo_list(_combo_list), padding(_padding) {}


  virtual void OnPaintItem(Canvas &canvas, const PixelRect rc,
                           unsigned i) override {
    canvas.DrawClippedText(rc.left + padding,
                           rc.top + padding, rc,
                           combo_list[i].StringValueFormatted);
  }
};

static const TCHAR*
OnItemHelp(unsigned i)
{
  if ((*ComboListPopup)[i].StringHelp)
    return (*ComboListPopup)[i].StringHelp;

  return _T("");
}

int
ComboPicker(const TCHAR *caption,
            const ComboList &combo_list,
            ListHelpCallback_t help_callback,
            bool enable_item_help)
{
  ComboListPopup = &combo_list;

  const UPixelScalar font_height =
    UIGlobals::GetDialogLook().text_font->GetHeight() + Layout::FastScale(2);
  const UPixelScalar max_height = Layout::GetMaximumControlHeight();
  const UPixelScalar row_height = font_height >= max_height
    ? font_height
    /* this formula is supposed to be a compromise between too small
       and too large: */
    : (font_height + max_height) / 2;

  const UPixelScalar padding = (row_height - font_height) / 2;

  ComboPickerSupport support(combo_list, padding);
  return ListPicker(caption,
                    combo_list.size(),
                    combo_list.ComboPopupItemSavedIndex,
                    row_height,
                    support, false,
                    help_callback,
                    enable_item_help ? OnItemHelp : NULL);
}

static void
OnHelpClicked(unsigned i)
{
  if (i < ComboListPopup->size()) {
    const ComboList::Item &item = (*ComboListPopup)[i];
    ComboPopupDataField->SetFromCombo(item.DataFieldIndex,
                                      item.StringValue);
  }

  wComboPopupWndProperty->OnHelp();
}

static int
ComboPicker(const WndProperty &control,
            const ComboList &combo_list, bool EnableItemHelp)
{
  return ComboPicker(control.GetCaption(), combo_list,
                     control.HasHelp() ? OnHelpClicked : nullptr,
                     EnableItemHelp);
}

bool
dlgComboPicker(WndProperty *theProperty)
{
  static bool bInComboPicker = false;
  // used to exit loop (optionally reruns combo with
  // lower/higher index of items for int/float
  bool bOpenCombo = true;

  // prevents multiple instances
  if (bInComboPicker)
    return false;

  bInComboPicker = true;

  StaticString<256> buffer;
  const TCHAR *reference = nullptr;

  while (bOpenCombo) {
    assert(theProperty != NULL);
    wComboPopupWndProperty = theProperty;

    ComboPopupDataField = wComboPopupWndProperty->GetDataField();
    assert(ComboPopupDataField != NULL);

    const ComboList combo_list = ComboPopupDataField->CreateComboList(reference);
    ComboListPopup = &combo_list;

    int idx = ComboPicker(*theProperty, combo_list,
                          ComboPopupDataField->GetItemHelpEnabled());

    bOpenCombo = false; //tell  combo to exit loop after close

    if (idx >= 0 && (unsigned)idx < combo_list.size()) {
      const ComboList::Item *item = &combo_list[idx];

      // OK/Select
      if (item->DataFieldIndex == ComboList::Item::NEXT_PAGE) {
        // we're last in list and the want more past end of list so select last real list item and reopen
        // we'll reopen, so don't call xcsoar data changed routine yet
        item = &combo_list[idx - 1];
        reference = buffer = item->StringValue;
        bOpenCombo = true; // reopen combo with new selected index at center
      } else if (item->DataFieldIndex == ComboList::Item::PREVIOUS_PAGE) {
        // same as above but lower items needed
        item = &combo_list[idx + 1];
        reference = buffer = item->StringValue;
        bOpenCombo = true;
      } else {
        ComboPopupDataField->SetFromCombo(item->DataFieldIndex,
                                          item->StringValue);
        wComboPopupWndProperty->RefreshDisplay();
      }
    } else {
      bInComboPicker = false;
      return false;
    }
  } // loop reopen combo if <<More>>  or <<Less>> picked

  bInComboPicker = false;
  return true;
}
