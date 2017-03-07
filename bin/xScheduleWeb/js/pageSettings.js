$(document).ready(function() {
  loadSettings();
  populateSideBar();
});

function populateSideBar() {
  if (uiSettings != undefined) {
    if (uiSettings.settings[0] == true) {
      $('#sideBar1').load('inc/sidebarNav.html');
    }
    if (uiSettings.settings[1] == true) {
      $('#sideBar2').load('inc/sidebarPlayerStatus.html');
    }
  }
}

//if enter
$(document).keypress(function(e) {
  if (e.which == 13) {
    updateSettings();
  }
});


function loadSettings() {
  if (uiSettings != undefined) {
    $('#webName').val(uiSettings.webName);
    $('#webColor').val(uiSettings.webColor);
    $('#notificationLevel').val(uiSettings.notificationLevel);
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

  var updatedSettings =
    `{
    "webName":"` + $('#webName').val() + `",
    "webColor":"` + $('#webColor').val() + `",
    "notificationLevel":"` + $('#notificationLevel').val() + `",
    "home":[` + $("#homeNav").prop("checked") + `, ` + $("#homeStatus").prop("checked") + `],
    "playlists":[` + $("#playlistNav").prop("checked") + `, ` + $("#playlistStatus").prop("checked") + `],
    "settings":[` + $("#settingsNav").prop("checked") + `, ` + $("#settingsStatus").prop("checked") + `]
    }`;

  updatedSettings.replace(/(\r\n|\n|\r)/gm, " ");
  storeKey('uiSettings', updatedSettings);
  loadUISettings();
}
