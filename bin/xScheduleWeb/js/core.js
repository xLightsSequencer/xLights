$(document).ready(function() {

  window.setInterval(function() {
    if (socket.readyState > '1') {
      updateStatus();
      updateNavStatus();
    }
  }, 1000);

  navLoadPlaylists();
  navLoadPlugins();
  checkLogInStatus();
  loadXyzzyData();

  //temp
  smartBrightness();
  smartVolume();

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
var playingStatus = `{status: "unknown"}`;

function updateStatus() {
  $.ajax({
    url: '/xScheduleQuery?Query=GetPlayingStatus',
    dataType: "json",
    success: function(response) {
      playingStatus = response;
    }
  });
}

var availableMatrices;
var xyzzyHighScore;

function loadXyzzyData() {
  //matrix
  $.ajax({
    type: "GET",
    url: '/xScheduleQuery?Query=GetMatrices',
    success: function(response) {
      availableMatrices = response;
    }
  });

  $.ajax({
    type: "GET",
    url: '/xyzzy?c=initialise',
    success: function(response) {
      xyzzyHighScore = JSON.parse('{"highscoreplayer":"' + response.highscoreplayer + '","highscore":' + response.highscore + '}');
    }

  });
}

var uiSettings;

function loadUISettings() {
  $.ajax({
    type: "GET",
    url: '/xScheduleStash?Command=Retrieve&Key=uiSettings',
    success: function(response) {

      if (response.result == "not logged in") {
        window.location.href = "login.html";
      } else {
        if (response.result == 'failed' && response.message == '') {
          var defaultSettings =
            `{
            "webName":"xLights Scheduler",
            "webColor":"#e74c3c",
            "notificationLevel":"1",
            "home":[true, false],
            "playlists":[true, true],
            "settings":[true, true],
            "navbuttons":[true, true, true, true, true, true, true]
          }`;
          storeKey('uiSettings', defaultSettings, '10');
          notification("Loaded Default Settings", "info", "0");
          uiSettings = JSON.parse(defaultSettings);
          if (getQueryVariable("plugin") == false) {
            populateSideBar();
          }
        } else {
          if (getQueryVariable("plugin") == false) {
            populateSideBar();
          }
          uiSettings = JSON.parse(response);
        }
        populateUI();
      }


    },
    error: function(response) {},
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
  if (socket.readyState <= '1') {
    socket.send('{"Type":"Login","Credential":"Log Out"}');
  } else {

    $.ajax({
      url: '/xScheduleLogin?Credential=logout',
      dataType: "json",
      success: function(response) {
        console.log(response);
        notification('Logged Out', 'success', '2');

      },
      error: function(error) {
        console.log("ERROR: " + error);
      }
    });
  }
}

function runCommand(name, param, reference) {

  //if websocket if open use that

  if (socket.readyState <= '1') {
    if (param == undefined) {
      if (reference == undefined) {
        socket.send('{"Type":"Command","Command":"' + name + '"}');
      } else {
        socket.send('{"Type":"Command","Command":"' + name + '","Reference":"' + reference + '"}');
      }
    } else {
      if (reference == undefined) {
        socket.send('{"Type":"Command","Command":"' + name + '","Parameters":"' + param + '"}');
      } else {
        socket.send('{"Type":"Command","Command":"' + name + '","Parameters":"' + param + '", "Reference":"' + reference + '"}');
      }
    }
  } else {

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


function storeKey(key, value, notificationLevel) {
  // written by cp16net so blame him.
  if (notificationLevel == undefined) {
    notificationLevel = "";
  }
  $.ajax({
    type: "POST",
    url: '/xScheduleStash?Command=Store&Key=' + key,
    data: value,
    success: function(response) {
      notification("Settings Saved", "success", notificationLevel);
      notification(response.result + response.message, "success", "2");
    },
    error: function(error) {
      notification("POST failed: ", response, '0');
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
  //console.log(uiSettings.notificationLevel);
  if (uiSettings == undefined || priority <= uiSettings.notificationLevel) {
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

//Upercase First Letter of string
function jsUcfirst(string) {
  return string.charAt(0).toUpperCase() + string.slice(1);
}

//sleep
function sleep(time) {
  return new Promise((resolve) => setTimeout(resolve, time));
}
