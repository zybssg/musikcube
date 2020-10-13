//////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2004-2019 musikcube team
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice,
//      this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//    * Neither the name of the author nor the names of other contributors may
//      be used to endorse or promote products derived from this software
//      without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////////

#include <stdafx.h>

#include "SettingsOverlays.h"

#include <musikcore/i18n/Locale.h>
#include <musikcore/support/Preferences.h>
#include <musikcore/support/PreferenceKeys.h>
#include <musikcore/library/ILibrary.h>

#include <cursespp/App.h>
#include <cursespp/SimpleScrollAdapter.h>
#include <cursespp/ListOverlay.h>
#include <cursespp/DialogOverlay.h>

using namespace musik;
using namespace musik::core;
using namespace musik::cube;
using namespace musik::cube::SettingsOverlays;
using namespace cursespp;

using Callback = std::function<void()>;

static void showNeedsRestart(Callback cb = Callback()) {
    std::shared_ptr<DialogOverlay> dialog(new DialogOverlay());

    (*dialog)
        .SetTitle(_TSTR("default_overlay_title"))
        .SetMessage(_TSTR("settings_needs_restart"))
        .AddButton("KEY_ENTER", "ENTER", _TSTR("button_ok"), [cb](std::string key) {
            if (cb) {
                cb();
            }
        });

    App::Overlays().Push(dialog);
}

static std::vector<std::string> allLocales;

void musik::cube::SettingsOverlays::ShowLocaleOverlay(std::function<void()> callback) {
    auto locale = i18n::Locale::Instance();

    using Adapter = cursespp::SimpleScrollAdapter;
    using ListOverlay = cursespp::ListOverlay;

    std::string currentLocale = locale.GetSelectedLocale();
    allLocales = locale.GetLocales();

    std::shared_ptr<Adapter> adapter(new Adapter());
    adapter->SetSelectable(true);

    int selectedIndex = 0;
    for (size_t i = 0; i < allLocales.size(); i++) {
        adapter->AddEntry(allLocales[i]);
        if (allLocales[i] == currentLocale) {
            selectedIndex = (int)i;
        }
    }

    std::shared_ptr<ListOverlay> dialog(new ListOverlay());

    dialog->SetAdapter(adapter)
        .SetTitle(_TSTR("locale_overlay_select_title"))
        .SetSelectedIndex(selectedIndex)
        .SetItemSelectedCallback(
            [callback, currentLocale]
            (ListOverlay* overlay, IScrollAdapterPtr adapter, size_t index) {
                if (allLocales[index] != currentLocale) {
                    i18n::Locale::Instance().SetSelectedLocale(allLocales[index]);
                    showNeedsRestart(callback);
                }
            });

    cursespp::App::Overlays().Push(dialog);
}

void musik::cube::SettingsOverlays::ShowLibraryTypeOverlay(std::function<void()> callback) {
    auto prefs = Preferences::ForComponent(prefs::components::Settings);

    auto libraryType = (ILibrary::Type) prefs->GetInt(
        prefs::keys::LibraryType, (int) ILibrary::Type::Local);

    using Adapter = cursespp::SimpleScrollAdapter;
    using ListOverlay = cursespp::ListOverlay;

    std::shared_ptr<Adapter> adapter(new Adapter());
    adapter->SetSelectable(true);
    adapter->AddEntry(_TSTR("settings_library_type_local"));
    adapter->AddEntry(_TSTR("settings_library_type_remote"));

    int selectedIndex = libraryType == ILibrary::Type::Local ? 0 : 1;

    std::shared_ptr<ListOverlay> dialog(new ListOverlay());

    dialog->SetAdapter(adapter)
        .SetTitle(_TSTR("settings_library_type_overlay_title"))
        .SetSelectedIndex(selectedIndex)
        .SetItemSelectedCallback(
            [prefs, callback]
            (ListOverlay* overlay, IScrollAdapterPtr adapter, size_t index) {
                auto updatedType = index == 0 ? ILibrary::Type::Local : ILibrary::Type::Remote;
                prefs->SetInt(prefs::keys::LibraryType, (int)updatedType);
                callback();
            });

    cursespp::App::Overlays().Push(dialog);
}