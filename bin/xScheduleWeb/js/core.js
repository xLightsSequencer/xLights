$(document).ready(function() {

  window.setInterval(function() {

    if (playingStatus != 'disabled') {
      updateStatus();
    }
    updateNavStatus();
  }, 1000);

  loadUISettings();
  navLoadPlaylists();
  navLoadPlugins();
  checkLogInStatus();

  //Add Hover effect to menus
  jQuery('ul.nav li.dropdown').hover(function() {
    jQuery(this).find('.dropdown-menu').stop(true, true).delay(200).fadeIn();
  }, function() {
    jQuery(this).find('.dropdown-menu').stop(true, true).delay(200).fadeOut();
  });


  // if mobile
  if (/Android|webOS|iPhone|iPad|iPod|BlackBerry/i.test(navigator.userAgent)) {
    $('#toggleButtonContainer').attr('class',
      'col-md-5 text-center nav-buttons');
    $('#nutcrackerLogo').attr('height',
      '30px');

  }

});
//global status
var playingStatus =
  `{status: "unknown"}`;

function updateStatus() {
  $.ajax({
    url: '/xScheduleQuery?Query=GetPlayingStatus',
    dataType: "json",
    success: function(response) {
      playingStatus = response;
    }
  });
}

var uiSettings;

function loadUISettings() {
  $.ajax({
    type: "GET",
    url: '/xScheduleStash?Command=Retrieve&Key=uiSettings',
    success: function(response) {
      uiSettings = JSON.parse(response);
      populateUI();
      if (getQueryVariable("plugin") == false) {
        populateSideBar();
      }
    },
    error: function(response) {
      var defaultSettings =
        `{
      "webName":"xLights Scheduler",
      "webColor":"#e74c3c",
      "notificationLevel":"1",
      "home":[true, false],
      "playlists":[true, true],
      "settings":[true, true]
      }`;
      storeKey('uiSettings', defaultSettings);
      uiSettings = JSON.parse(defaultSettings);
      populateUI();
      if (getQueryVariable("plugins") == false) {
        populateSideBar();
      }
    },
  });
}

function populateUI() {
  $('#headerName').html(uiSettings.webName);
  $('#nav').css("background-color", uiSettings.webColor);
  $('#nav').css("border-color", uiSettings.webColor);
  document.title = uiSettings.webName;
}

function checkLogInStatus() {
  if (playingStatus.status == 'unknown') {
    $.ajax({
      url: '/xScheduleQuery?Query=GetPlayingStatus',
      dataType: "json",
      success: function(response) {
        if (response.result == "not logged in") {
          window.location.href = "login.html";
        }
      }

    });
  } else if (playingStatus.result == "not logged in") {
    window.location.href = "login.html";
  }


}


function navLoadPlaylists() {
  $.ajax({
    url: '/xScheduleQuery?Query=GetPlayLists',
    dataType: "json",
    success: function(response) {
      for (var i = 0; i < response.playlists.length; i++) {
        var navPlaylists =
          `<li><a onclick="updatePage('page', 'playlists','` +
          response.playlists[
            i].name + `')">` + response.playlists[i].name +
          `</a></li>`;
        $("#playlists ul").append(navPlaylists);
      }
    }
  });
}

function navLoadPlugins() {
  $.ajax({
    url: '/xScheduleQuery?Query=ListWebFolders&Parameters=Plugins',
    dataType: "json",
    success: function(response) {
      for (var i = 0; i < response.folders.length; i++) {
        var navPlugins =
          `<li><a onclick="updatePage('plugin', '` + response.folders[
            i] +
          `')">` + response.folders[i] + `</a></li>`;
        $("#plugins ul").append(navPlugins);
      }
    }
  });
}

function logout() {
  $.ajax({
    url: '/xScheduleLogin?Credential=logout',
    dataType: "json",
    success: function(response) {
      notification('Logged Out', 'success', '2');

    },
    error: function(error) {
      console.log("ERROR: " + error);
    }
  });
}

function runCommand(name, param) {
  if (param == undefined)
    var command = name
  else
    var command = name + '&Parameters=' + param

  $.ajax({
    url: '/xScheduleCommand?Command=' + command,
    success: function(response) {
      if (response.result == 'ok')
        notification(response.result + ': ' + response.message, 'success', '2');
      if (response.result == 'failed')
        notification('Failed: ' + response.message, 'danger', '0');
    },
    error: function(response) {
      notification(response.result + ': ' + response.message, 'danger', '1');
    }
  });
}

function findPercent(length, left) {
  var secLength = parseInt(length.split(":")[0]) * 60 + parseInt(length
    .split(
      ":")[1]);
  var secLeft = parseInt(left.split(":")[0]) * 60 + parseInt(left.split(
    ":")[1]);
  var percent = (secLength - secLeft) / secLength * 100;
  return Math.round(percent) + "%";
}

function updateNavStatus() {
  checkLogInStatus();

  if (playingStatus.status == 'idle') {

    $('#random').attr('disabled', 'disabled');
    $('#steplooping').attr('disabled', 'disabled');
    $('#playlistlooping').attr('disabled', 'disabled');
  } else {
    $('#random').removeAttr("disabled");
    $('#steplooping').removeAttr("disabled");
    $('#playlistlooping').removeAttr("disabled");

    if (playingStatus['random'] == "false") {
      $('#random').attr('class',
        "btn btn-default glyphicon glyphicon-random");
    } else if (playingStatus['random'] == "true") {
      $('#random').attr('class',
        "btn btn-info glyphicon glyphicon-random");
    }
    if (playingStatus['steplooping'] == "false") {
      $('#steplooping').attr('class',
        "btn btn-default glyphicon glyphicon-repeat");
    } else if (playingStatus['steplooping'] == "true") {
      $('#steplooping').attr('class',
        "btn btn-info glyphicon glyphicon-repeat");
    }
    if (playingStatus['playlistlooping'] == "false") {
      $('#playlistlooping').attr('class',
        "btn btn-default glyphicon glyphicon-refresh");
    } else if (playingStatus['playlistlooping'] == "true") {
      $('#playlistlooping').attr('class',
        "btn btn-info glyphicon glyphicon-refresh");
    }
  }
  //output to lights
  if (playingStatus['outputtolights'] == 'false') {
    $('#outputtolights').attr('class',
      "btn btn-danger glyphicon glyphicon-eye-close");
  } else if (playingStatus['outputtolights'] == 'true') {
    $('#outputtolights').attr('class',
      "btn btn-success glyphicon glyphicon-eye-open");
  }

}

function storeKey(key, value) {
  // written by cp16net so blame him.
  $.ajax({
    type: "POST",
    url: '/xScheduleStash?Command=Store&Key=' + key,
    data: value,
    success: function(response) {
      notification("Settings Saved", "success", "0");
      notification(response.result + response.message, "success", "2");
    },
    error: function(error) {
      console.log("ERROR: " + error);
    }
  });
}

function retrieveKey(key, f) {
  // written by cp16net so blame him.
  $.ajax({
    type: "GET",
    url: '/xScheduleStash?Command=Retrieve&Key=' + key,
    success: f,
    error: f,
    // error: function(error) {
    //   console.log("ERROR: " + error);

  });
}

function notification(message, color, priority) {

  // priority   //
  // "" = always //
  // 1 = debug   //
  console.log(uiSettings.notificationLevel);
  if (priority <= uiSettings.notificationLevel) {
    $.notify({
      // options
      message: message
    }, {
      // settings
      type: color
    });
  }
}

function findPercent(length, left) {
  var secLength = parseInt(length.split(":")[0]) * 60 + parseInt(length.split(
    ":")[1]);
  var secLeft = parseInt(left.split(":")[0]) * 60 + parseInt(left.split(":")[1]);
  var percent = (secLength - secLeft) / secLength * 100;
  return Math.round(percent) + "%";
}
