$(document).ready(function() {
  var i = "0";
  var AJAXTimer = window.setInterval(function() {
    if (socket.readyState > '1') {
      updateStatus();
      updateNavStatus();
    }
    if (socket.readyState < '2') {
      i = i++;
      if (i = '3') {
        clearInterval(AJAXTimer);
      }
    }
  }, 1000);

  loadUISettings();
  navLoadPlaylists();
  navLoadPlugins();
  checkLogInStatus();
  loadXyzzyData();
  loadXyzzy2Data();


  // //Add Hover effect to menus
  // jQuery('ul.nav li.dropdown').hover(function() {
  //   jQuery(this).find('.dropdown-menu').stop(true, true).delay(200).fadeIn();
  // }, function() {
  //   jQuery(this).find('.dropdown-menu').stop(true, true).delay(200).fadeOut();
  // });


  // if mobile
  if (/Android|webOS|iPhone|iPad|iPod|BlackBerry/i.test(navigator.userAgent)) {
    $('#toggleButtonContainer').attr('class',
      'col-md-5 text-center nav-buttons');
    $('#nutcrackerLogo').attr('height',
      '30px');
    isMobile = true;

  }

});
var isMobile;
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
var xyzzy2HighScore;
var uiSettings;

function loadUISettings(resetToDefault) {
  $.ajax({
    type: "GET",
    url: '/xScheduleStash?Command=Retrieve&Key=uiSettings',
    success: function(response) {

      if (response.result == "not logged in") {
        window.location.href = "login.html";
      } else {
        if (response.result == 'failed' && response.message == '' || resetToDefault == true) {
          var defaultSettings = {
            "webName": "xLights Scheduler",
            "webColor": "#004800",
            "notificationLevel": "1",
            "pages": [{
              "page": "home",
              "values": [true, false]
            }, {
              "page": "playlists",
              "values": [true, true]
            }, {
              "page": "settings",
              "values": [true, true]
            }],
            "navbuttons": [{
              "random": true,
              "steplooping": false,
              "playlistlooping": true,
              "volumeMute": true,
              "brightnessLevel": false,
              "outputtolights": true,
              "toggleMute": false
            }]
          };

          storeKey('uiSettings', JSON.stringify(defaultSettings), '10');
          notification("Loaded Default Settings", "info", "0");
          uiSettings = defaultSettings;
          if (getQueryVariable("plugin") == false)
            populateSideBar();

          if (currentPage == 'settings')
            loadSettings();
          window.scrollTo(0, 0);
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

function getPage(settings, pageName) {
  for (var page in settings.pages) {
    if (settings.pages[page].page == pageName) {
      return settings.pages[page];
    }
  }
}

// function getToggleButton(settings, buttonName) {
//   for (var page in settings.navbuttons) {
//     if (settings.navbuttons[page].page == pageName) {
//       return settings.navbuttons[page];
//     }
//   }
// }


function populateUI() {
  $('#headerName').html(uiSettings.webName);
  $('#nav').css("background-color", uiSettings.webColor);
  document.title = uiSettings.webName;
  $('#toggleButtonContainer').html("");
  //Nav Buttons
  if (uiSettings.navbuttons[0].random == true)
    $('#toggleButtonContainer').append(`<button onclick="runCommand('Toggle current playlist random')" id="random" title="Randomize Playlist" class="btn btn-default glyphicon glyphicon-random" disabled></button>`);
  if (uiSettings.navbuttons[0].steplooping == true)
    $('#toggleButtonContainer').append(`<button onclick="runCommand('Toggle loop current step')" id="steplooping" title="Loop Current Step" class="btn btn-default glyphicon glyphicon-repeat" disabled></button>`);
  if (uiSettings.navbuttons[0].playlistlooping == true)
    $('#toggleButtonContainer').append(`<button onclick="runCommand('Toggle current playlist loop')" id="playlistlooping" title="Loop Playlist" class="btn btn-default glyphicon glyphicon-refresh " disabled></button>`);
  if (uiSettings.navbuttons[0].volumeMute == true && isMobile != true) {
    $('#toggleButtonContainer').append(`<button id="volumeMute" title="Volume Control" class="btn btn-default glyphicon glyphicon-volume-off"></button>`);
    smartVolume();
  }
  if (uiSettings.navbuttons[0].brightnessLevel == true && isMobile != true) {
    $('#toggleButtonContainer').append(`<button id="brightnessLevel" title="Adjust Brightness" class="btn btn-default glyphicon glyphicon-flash"></button>`);
    smartBrightness();
  }
  if (uiSettings.navbuttons[0].outputtolights == true)
    $('#toggleButtonContainer').append(`<button onclick="runCommand('Toggle output to lights')"" class="btn btn-danger glyphicon glyphicon-eye-open" id="outputtolights" title="Toggle Ouput to lights" type="button"></button>`);
  if (uiSettings.navbuttons[0].toggleMute == true)
    $('#toggleButtonContainer').append(`<button onclick="runCommand('Toggle mute')" class="btn btn-danger glyphicon glyphicon-eye-open" id="toggleMute"  title="Toggle Mute " type="button"></button>`);

  //help button
  $('#toggleButtonContainer').append(`<button class="btn btn-info glyphicon glyphicon-info-sign" type="button" title="Click for assistance" data-toggle="modal" data-target="#help"></button>`);
  //Populate Button State
  updateNavStatus();
  populateSideBar();
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
    console.log("xJAX CALL");
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
    url: '/xyzzy?c=highscore',
    success: function(response) {
      xyzzyHighScore = JSON.parse('{"highscoreplayer":"' + response.highscoreplayer + '","highscore":' + response.highscore + '}');
    }

  });
}

function loadXyzzy2Data() {
  $.ajax({
    type: "GET",
    url: '/xyzzy2?c=highscore',
    success: function(response) {
      xyzzy2HighScore = JSON.parse('{"highscoreplayer":"' + response.highscoreplayer + '","highscore":' + response.highscore + '}');
    }

  });
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
