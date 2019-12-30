$("#volume").slider();
$("#volume").on("slide", function(slideEvt) {
  $("#volumeSliderVal").text(slideEvt.value);
});
$("#brightness").slider();
$("#brightness").on("slide", function(slideEvt) {
  $("#brightnessSliderVal").text(slideEvt.value);
  console.log(slideEvt.value);
});
$(document).ready(function() {
  window.setInterval(function() {
    checkForUpdate();
  }, 100);

  if (/Android|webOS|iPhone|iPad|iPod|BlackBerry/i.test(navigator.userAgent)) {
    $('#sideNav').html("");
  }
  populateSideBar();

});

var oldPlayingStatus = '';

function checkForUpdate() {
  //update Clock begining
  $('#homeTimeContainer').html(playingStatus.time);

  if (oldPlayingStatus.status != playingStatus.status) {
    //status changed redraw page
    if (playingStatus.status == 'idle') {
      populateTableIdle();
      $("#controlButtonContainer").html("");
    } else if (playingStatus.status == 'playing') {

      if (oldPlayingStatus.status != 'paused') {
        populateTablePlaying();
      }

      var toggleButtons =
        `<div class="span6 text-center">
      <button onclick="runCommand('Prior step in current playlist')" class="btn btn-default glyphicon glyphicon-backward" title='Back' type="button"></button>
      <button id="buttonPlayPause" onclick="runCommand('Pause')" class="btn btn-default glyphicon glyphicon-pause" title='Pause' type="button"></button>
      <button onclick="runCommand('Stop all now')" class="btn btn-default glyphicon glyphicon-stop" title='Stop All' ype="button"></button>
      <button onclick="runCommand('Next step in current playlist')" class="btn btn-default glyphicon glyphicon-forward" title='Next' type="button"></button>
      </div>`;
      $("#controlButtonContainer").html(toggleButtons);


    } else if (playingStatus.status == 'paused') {
      if ($("#playlistItems").html() == undefined) {
        populateTablePlaying();
      }
      var toggleButtons =
        `<div class="span6 text-center">
      <button onclick="runCommand('Prior step in current playlist')" class="btn btn-default glyphicon glyphicon-backward" title='Back' type="button"></button>
      <button id="buttonPlayPause" onclick="runCommand('Pause')" class="btn btn-default glyphicon glyphicon-play" title='Pause' type="button"></button>
      <button onclick="runCommand('Stop all now')" class="btn btn-default glyphicon glyphicon-stop" title='Stop All' ype="button"></button>
      <button onclick="runCommand('Next step in current playlist')" class="btn btn-default glyphicon glyphicon-forward" title='Next' type="button"></button>
      </div>`;
      $("#controlButtonContainer").html(toggleButtons);

    }
    dashboardLoadStatus();
  }

  if (playingStatus.status == 'playing') {
    //update "Playing status" box
    dashboardLoadStatus();

    if (oldPlayingStatus.stepid != playingStatus.stepid) {
      $('#table' + oldPlayingStatus.stepid).removeClass('success');
      $('#table' + oldPlayingStatus.stepid).removeClass('warning');
      $('#table' + playingStatus.stepid).addClass('success');
      $('#table' + oldPlayingStatus.nextstepid).removeClass('warning');
      $('#table' + playingStatus.nextstepid).addClass('warning');
    }
  }
  oldPlayingStatus = playingStatus;
}

function dashboardLoadStatus() {

  if (playingStatus.status == "idle") {
    //Populate Idle Page
    $.ajax({
      url: '/xScheduleQuery?Query=GetNextScheduledPlayList',
      dataType: "json",
      indexValue: status,
      success: function(response) {
        if (response.start == "Never") {
          var notPlaying = '<p><center>Show Not Running!</center></p>';
          $('#currentStep').html(notPlaying);
        } else {

          var awaitingSchedule = `
          <p><h2>Schedule: ` + response.schedulename + `</h2></p
          <p><h4>Playlist: ` + response.playlistname + `</h4></p>
          <p>Schedule Start: ` + response.start + `</p>
          <p>Time End: ` + response.end + `</p>
          </p>

          `;
          $('#currentStep').html(awaitingSchedule);

          if (response.start == "NOW!") {
            var restartButton = `<hr><p><b>Looks like your schedule should be running, please restart!</b></p><button onclick="runCommand('Restart playlist schedules','id:` + response.playlistid + `')" class="btn btn-warning glyphicon glyphicon-triangle-right" title='Back' type="button">Restart</button>`;
            $("#controlButtonContainer").html(restartButton);
            console.log("Button Loaded");
          }
        }

        console.log(response);
        //$("#controlButtonContainer").html("");
      }
    });


  } else {
    //playing

    var currentPlaylist =
      `<span class="icon"><i class="icon-file"></i></span><h5>Playlist: ` +
      playingStatus.playlist + `</h5>`
    var Playing = `
    <h3>` + playingStatus.step + `</h3><h5>Duration: ` + playingStatus.length.split(".")[0] + `</h5>
      <div class="progress">
  <div class="progress-bar" role="progressbar" aria-valuenow="60" aria-valuemin="0" aria-valuemax="100" style="width: ` + findPercent(playingStatus.length, playingStatus.left) + `;">
    ` + playingStatus.left.split(".")[0] + `
  </div>
</div>
<h4>Next: ` + playingStatus.nextstep + `</h4>
      `;

    $("#currentStep").html(Playing);
    //Set bar
    // $("#currentPlaylistLoadingBar").css("width", findPercent(
    //   playingStatus
    //   .length.split(".")[0], playingStatus.left.split(".")[0]));
    //updatePlaylistSteps(playingStatus.playlist, playingStatus.step);
  }
}



function populateTableIdle(status, query) {

  $.ajax({
    url: '/xScheduleQuery?Query=GetPlayLists',
    dataType: "json",
    indexValue: status,
    success: function(response) {
      populatePlaylistBox('playlists', 'Available Playlists');

      for (var i = 0; i < response.playlists.length; i++) {


        var idle =
          `<tr id='table` + response.playlists[i].id + `'>
                <td class="col-md-8">` + response.playlists[i].name + `</td>
                <td class="col-md-3">` + response.playlists[i].nextscheduled + `</td>
                <td class="col-md-1">` + response.playlists[i].length.split(".")[0] + `</td>
                <td class="col-md-2">
                  <button type="button" onclick="runCommand('Play specified playlist', 'id:` + response.playlists[i].id + `')" class="btn btn-info btn-xs" name="button">Play</button>
                  <button type="button" onclick="updatePage('page', 'playlists','` + response.playlists[i].name + `')" class="btn btn-default btn-xs" title='More info' name="button">View</button>
                </td>
             </tr>`;
        $("#playlistItems").append(idle);
      }

      // load Table Logic


      $('#tablePlaylists').dataTable({
        "ordering": false,
        "searching": false,
      });


    }
  });
}

function populateTablePlaying() {

  $.ajax({
    url: '/xScheduleQuery?Query=GetPlayListSteps&Parameters=' + playingStatus.playlist,
    dataType: "json",
    success: function(response) {

	  if (playingStatus.scheduleend == 'N/A') {
	    populatePlaylistBox('steps', playingStatus.playlist + ': ');
	  }
	  else {
	    populatePlaylistBox('steps', playingStatus.playlist + ': until ' + playingStatus.scheduleend);
	  }
	  
      for (var i = 0; i < response.steps.length; i++) {
        var playing =
          `<tr id='table` + response.steps[i].id + `'>
              <td class="col-md-8">` + response.steps[i].name + `</td>
              <td class="col-md-1">` + response.steps[i].length.split(".")[0] + `</td>
              <td class="col-md-2">
                  <button type="button" onclick="runCommand('Play playlist starting at step', 'id:` + playingStatus.playlistid + `,id:` + response.steps[i].id + `')" class="btn btn-info btn-xs " name="button" title="Jump to this step">Play</button>
                   <!--<button type="button" class="btn btn-default btn-xs glyphicon glyphicon-plus" onclick="runCommand('Enqueue playlist step', '` + playingStatus.playlist + `,` + response.steps[i].name + `')" name="button" title="Queue step next"></button
              </td>
           </tr>`;
        $("#playlistItems").append(playing);
      }
      // load Table Logic
      $('#tableSteps').dataTable({
        "ordering": false,
        "searching": false,
      });


    }
  });
}

var statusTable = '';

function populatePlaylistBox(type, title) {

  var playlists =
    `<div class="panel panel-default">
      <div class="panel-heading main-color-bg">
        <h3 id="currentPlaylist" id="playlistTableTitle" class="panel-title">` + title + `</h3>
      </div>
      <div class="panel-body">
        <table id="tablePlaylists" class="table table-striped table-hover  table-bordered">
          <thead>
            <tr>
              <th class="col-md-8">Name</th>
              <th class="col-md-3">Schedule</th>
              <th class="col-md-1">Length</th>
              <th class="col-md-2">Actions</th>
            </tr>
          </thead>
          <tbody id="playlistItems">
          </tbody>
          </table>
      </div>
    </div>`;

  var steps =
    `<div class="panel panel-default">
        <div class="panel-heading main-color-bg">
          <h3 id="currentPlaylist" id="playlistTableTitle" class="panel-title">` + title + `</h3>
        </div>
        <div class="panel-body">
          <table id="tableSteps" class="table table-striped table-hover  table-bordered">
            <thead>
              <tr>
                <th class="col-md-8">Name</th>
                <th class="col-md-1">Length</th>
                <th class="col-md-2">Actions</th>
              </tr>
            </thead>
            <tbody id="playlistItems">
            </tbody>
            </table>
        </div>
      </div>`;
  if (type == 'playlists') {
    $("#playlistsTable").html(playlists);
  } else if (type == 'steps') {
    $("#playlistsTable").html(steps);
  }

}
