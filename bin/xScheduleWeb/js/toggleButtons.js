var volumePOS;
var volumelastY;
var volumeinterval;
var volumeArray = ["RESET"];

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

  if (playingStatus['volume'] == "0") {
    $('#toggleMute').attr('class', "btn btn-danger glyphicon glyphicon-volume-off");
    $('#toggleMute').html("");
  } else if (playingStatus['volume'] > "0") {
    if (playingStatus['volume'] == 100) {
      $('#toggleMute').attr('class', "btn btn-success glyphicon glyphicon-volume-up");
      $('#toggleMute').html("");
    } else {
      $('#toggleMute').attr('class', "btn btn-success glyphicon glyphicon-volume-down");
      $('#toggleMute').html((0 + playingStatus['volume']).slice(-2));
    }

  }

  if (playingStatus['volume'] == "0") {
    $('#volumeMute').attr('class', "btn btn-danger glyphicon glyphicon-volume-off");
    $('#volumeMute').html("");
  } else if (playingStatus['volume'] > "0") {
    //display test
    if (playingStatus['volume'] == 100) {
      $('#volumeMute').attr('class', "btn btn-success glyphicon glyphicon-volume-up");
      $('#volumeMute').html("");
    } else {
      $('#volumeMute').attr('class', "btn btn-success glyphicon glyphicon-volume-down");
      $('#volumeMute').html((0 + playingStatus['volume']).slice(-2));
    }

  }

  if (playingStatus['brightness'] == "0") {
    $('#brightnessLevel').attr('class', "btn btn-danger glyphicon glyphicon-flash");
    $('#brightnessLevel').html("");
  } else if (playingStatus['brightness'] > "0") {
    //display test
    if (playingStatus['brightness'] == 100) {
      $('#brightnessLevel').attr('class', "btn btn-default glyphicon glyphicon-flash");
      $('#brightnessLevel').html("");
    } else {
      $('#brightnessLevel').attr('class', "btn btn-default glyphicon glyphicon-flash");
      $('#brightnessLevel').html((0 + playingStatus['brightness']).slice(-2));
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

  //update xlights version
  $("#version").html("xScheduler " + playingStatus['version']);


}

//SMART

function smartVolume() {
  $("#volumeMute").mousedown(function(evt) {
    var e = $("#volumeMute").mousemove();
    offset = $(this).offset();
    volumelastY = Math.round(100 - ((evt.pageY - offset.top) * 100 / 33));
    volumeinterval = window.setInterval(volumeDrag, 200, e, offset);
  }).mouseup(function(evt) {
    clearInterval(volumeinterval);
  });

  $("#volumeMute").mousemove(function handler(evt) {
    var offset = $(this).offset();
    volumePOS = Math.round(100 - ((evt.pageY - offset.top) * 100 / 33));
  });

  $("#volumeMute").mouseup(function handler(evt) {
    var offset = $(this).offset();
    var currentY = Math.round(100 - ((evt.pageY - offset.top) * 100 / 33));
    if (volumelastY == currentY) {
      //toggle mute
      runCommand('Toggle mute');
    }
  });

  function volumeDrag() {
    volumeArray.push(volumePOS);
    for (var i = volumeArray.length; i > 10; i--) {
      volumeArray.shift();
    }
    if (volumeArray.every((val, i, arr) => val == arr[0]) == true) {
      volumeArray = ["RESET"];
      clearInterval(volumeinterval);
    }
    if (isNaN(volumePOS) == false && volumeArray.length > '1') {
      runCommand('Set volume to', volumePOS)
    }

  }
}

var brightnessPOS;
var brightnesslastY;
var brightnessinterval;
var brightnessArray = ["RESET"];

function smartBrightness() {
  $("#brightnessLevel").mousedown(function(evt) {
    var e = $("#brightnessLevel").mousemove();
    offset = $(this).offset();
    brightnesslastY = Math.round(100 - ((evt.pageY - offset.top) * 100 / 33));
    brightnessinterval = window.setInterval(brightnessDrag, 100, e, offset);
  }).mouseup(function(evt) {
    clearInterval(brightnessinterval);
  });

  $("#brightnessLevel").mousemove(function handler(evt) {
    var offset = $(this).offset();
    brightnessPOS = Math.round(100 - ((evt.pageY - offset.top) * 100 / 33));
  });

  $("#brightnessLevel").mouseup(function handler(evt) {
    var offset = $(this).offset();
    var currentY = Math.round(100 - ((evt.pageY - offset.top) * 100 / 33));
    if (brightnesslastY == currentY) {
      //toggle mute
      //runCommand('Toggle mute');
    }
  });

  function brightnessDrag() {
    brightnessArray.push(brightnessPOS);
    for (var i = brightnessArray.length; i > 10; i--) {
      brightnessArray.shift();
    }
    if (brightnessArray.every((val, i, arr) => val == arr[0]) == true) {
      brightnessArray = ["RESET"];
      clearInterval(brightnessinterval);
    }
    if (isNaN(brightnessPOS) == false && brightnessArray.length > '1') {
      runCommand('Set brightness to n%', brightnessPOS)
    }

  }
}
