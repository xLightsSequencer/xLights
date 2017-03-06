$(document).ready(function() {
  loadSettings();
  populateSideBar();
});

function populateSideBar() {
  if (uiSettings != undefined) {
    if (uiSettings.settings[0] == true) {
      $('#sideBar2').load('inc/sidebarNav.html');
    }
    if (uiSettings.settings[1] == true) {
      $('#sideBar1').load('inc/sidebarPlayerStatus.html');
    }
  }
}


function loadSettings() {
  if (uiSettings != undefined) {
    $('#webName').val(uiSettings.webName);
    $('#webColor').val(uiSettings.webColor);
    //home
    $("#homeNav").prop("checked", uiSettings.home[0]);
    $("#homeStatus").prop("checked", uiSettings.home[1]);
    //playlists
    $("#playlistNav").prop("checked", uiSettings.playlists[0]);
    $("#playlistStatus").prop("checked", uiSettings.playlists[1]);
    //settings
    $("#settingsNav").prop("checked", uiSettings.settings[0]);
    $("#settingsStatus").prop("checked", uiSettings.settings[1]);
  }
}


function updateSettings() {

  var defaultSettings =
    `{
    "webName":"` + $('#webName').val() + `",
    "webColor":"` + $('#webColor').val() + `",
    "home":[` + $("#homeNav").prop("checked") + `, ` + $("#homeStatus").prop("checked") + `],
    "playlists":[` + $("#playlistNav").prop("checked") + `, ` + $("#playlistStatus").prop("checked") + `],
    "settings":[` + $("#settingsNav").prop("checked") + `, ` + $("#settingsStatus").prop("checked") + `]
    }`;

  defaultSettings.replace(/(\r\n|\n|\r)/gm, " ");
  storeKey('uiSettings', defaultSettings);
  uiSettings = defaultSettings;
  loadUISettings();
}
