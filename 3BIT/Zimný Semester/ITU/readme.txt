Adresárová štruktúra s autorstvom súborov:

Vstup do programu: 
└── lib/main.dart

@@@ Hugo Bohácsek, xbohach00
TUI Frontend
ux_clicker/
├── lib/models/tui/game_state.dart
├── lib/views/tui/react_minigame.dart
└── lib/views/tui/tui_screen.dart

@@@ Štefan Dubnicka, xdubnis00
Backend
ux_clicker/
└── lib/services/server.dart

Mac System 7 Frontend
ux_clicker/
├── lib/views/mac_system7/mac_system7_cors_minigame.dart
├── lib/views/mac_system7/mac_system7_draggable_window.dart
├── lib/views/mac_system7/mac_system7_mainscreen.dart
├── lib/views/mac_system7/mac_system7_mini_game_card.dart
├── lib/views/mac_system7/mac_system7_upgrade_card.dart
├── lib/views/mac_system7/mac_system7_upgrade_column.dart
├── lib/views/mac_system7/mac_system7_upgrade_offer_row.dart
├── lib/views/mac_system7/mac_system7_upgrade_shop_screen.dart
├── lib/repositories/gamerepository.dart
├── lib/controllers/mac_system7/*.dart
└── lib/models/mac_system7/*.dart

@@@ Filip Jenis, xjenisf00
Windows 11 Frontend
ux_clicker/
├── lib/controllers/w11/clickergame.dart
├── lib/controllers/w11/upgradeshop.dart
├── lib/controllers/w11/winapi.dart
├── lib/models/w11/clickergamestate.dart
├── lib/models/w11/dragable_payload.dart
├── lib/models/w11/minigame.dart
├── lib/models/w11/upgrade.dart
├── lib/models/w11/upgradeoffer.dart
├── lib/models/w11/winapimessage.dart
├── lib/models/w11/winapiscreen.dart
├── lib/views/w11/widgets/bonuspointschip.dart
├── lib/views/w11/widgets/minigamecard.dart
├── lib/views/w11/widgets/sidedrawer.dart
├── lib/views/w11/widgets/upgradecard.dart
├── lib/views/w11/widgets/upgradeofferrow.dart
├── lib/views/w11/widgets/upgradescolumn.dart
├── lib/views/w11/widgets/winapimessage.dart
├── lib/views/w11/widgets/winapiscreencard.dart
├── lib/views/w11/mainscreen.dart
├── lib/views/w11/upgradeshopscreen.dart
└── lib/views/w11/winapiscreen.dart