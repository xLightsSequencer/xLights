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
      console.log(getQueryVariable("plugins"));
      if (getQueryVariable("plugins") == false) {
        populateSideBar();
      }
    },
    error: function(response) {
      var defaultSettings =
        `{
    "webName":"xLights Scheduler",
    "webColor":"#e74c3c",
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
    success: function(response) {},
    error: function(error) {
      console.log("ERROR: " + error);
    }
  });
}

function runCommand(name, param) {
  if (param == undefined) {
    $.ajax({
      url: '/xScheduleCommand?Command=' + name,
      error: function(response) {
        jAlert('This is a custom alert box', 'Alert Dialog');
      }
    });
  } else {
    $.ajax({
      url: '/xScheduleCommand?Command=' + name + '&Parameters=' +
        param,
      success: function(response) {}
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

function updateNavStatus() {
  checkLogInStatus();

  if (playingStatus.status == 'idle') {

    $('#random').attr('class',
      "btn btn-default glyphicon glyphicon-random disabled"
    );
    $('#steplooping').attr('class',
      "btn btn-default glyphicon glyphicon-repeat disabled"
    );
    $('#playlistlooping').attr('class',
      "btn btn-default glyphicon glyphicon-refresh disabled"
    );
  } else {

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
    success: function(response) {},
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
