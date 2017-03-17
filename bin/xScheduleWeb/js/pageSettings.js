$(document).ready(function() {
  loadSettings();
  populateSideBar();

  $("#toggleMuteSetting").mouseup(function() {
    if ($("#smartVolumeSetting").prop("checked") == true)
      $("#smartVolumeSetting").prop("checked", false);
  });
  $("#smartVolumeSetting").mouseup(function() {
    if ($("#toggleMuteSetting").prop("checked") == true)
      $("#toggleMuteSetting").prop("checked", false);
  });
});

function populateSideBar() {
  if (uiSettings != undefined) {
    if (uiSettings.settings[0] == true) {
      $('#sideBar1').load('inc/sidebarNav.html');
    }
    if (uiSettings.settings[1] == true) {
      $('#sideBar2').load('inc/sidebarPlayerStatus.html');
    }
  } else {
    sleep(100),
      function() {
        populateSideBar();
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
    $('#webNameSetting').val(uiSettings.webName);
    $('#webColorSetting').val(uiSettings.webColor);
    $('#notificationLevelSetting').val(uiSettings.notificationLevel);
    //home
    $("#homeNavSetting").prop("checked", uiSettings.home[0]);
    $("#homeStatusSetting").prop("checked", uiSettings.home[1]);
    //playlists
    $("#playlistNavSetting").prop("checked", uiSettings.playlists[0]);
    $("#playlistStatusSetting").prop("checked", uiSettings.playlists[1]);
    //settings
    $("#settingsNavSetting").prop("checked", uiSettings.settings[0]);
    $("#settingsStatusSetting").prop("checked", uiSettings.settings[1]);
    //nav buttons
    $("#smartVolumeSetting").prop("checked", uiSettings.navbuttons[0]);
    $("#smartBrightnessSetting").prop("checked", uiSettings.navbuttons[1]);
    $("#outputToLightsSetting").prop("checked", uiSettings.navbuttons[2]);
    $("#repeatPlaylistSetting").prop("checked", uiSettings.navbuttons[3]);
    $("#repeatStepsSetting").prop("checked", uiSettings.navbuttons[4]);
    $("#toggleRandomSetting").prop("checked", uiSettings.navbuttons[5]);
    $("#toggleMuteSetting").prop("checked", uiSettings.navbuttons[6]);
  } else {
    sleep(100),
      function() {
        loadSettings();
      }
  }
}


function updateSettings() {

  var updatedSettings =
    `{
    "webName":"` + $('#webNameSetting').val() + `",
    "webColor":"` + $('#webColorSetting').val() + `",
    "notificationLevel":"` + $('#notificationLevelSetting').val() + `",
    "home":[` + $("#homeNavSetting").prop("checked") + `, ` + $("#homeStatusSetting").prop("checked") + `],
    "playlists":[` + $("#playlistNavSetting").prop("checked") + `, ` + $("#playlistStatusSetting").prop("checked") + `],
    "settings":[` + $("#settingsNavSetting").prop("checked") + `, ` + $("#settingsStatusSetting").prop("checked") + `],
    "navbuttons":[` + $("#smartVolumeSetting").prop("checked") + `,` + $("#smartBrightnessSetting").prop("checked") + `,` + $("#outputToLightsSetting").prop("checked") +
    `,` + $("#repeatPlaylistSetting").prop("checked") + `,` + $("#repeatStepsSetting").prop("checked") + `,` + $("#toggleRandomSetting").prop("checked") + `,` + $("#toggleMuteSetting").prop("checked") + `]
    }`;
  //Save settings, Update UI
  updatedSettings.replace(/(\r\n|\n|\r)/gm, " ");
  storeKey('uiSettings', updatedSettings);
  uiSettings = JSON.parse(updatedSettings);
  populateUI();
  populateSideBar();

}
