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
  }, 1000);

  if (/Android|webOS|iPhone|iPad|iPod|BlackBerry/i.test(navigator.userAgent)) {
    $('#sideNav').html("");
  }
  populateSideBar();


});



function populateSideBar() {
  if (uiSettings != undefined) {
    if (uiSettings.home[0] == true) {
      $('#sideBar1').load('inc/sidebarNav.html');
    }
    if (uiSettings.home[1] == true) {
      $('#sideBar2').load('inc/sidebarPlayerStatus.html');
    }
  }
}

var oldPlayingStatus = '';

function checkForUpdate() {
  //update Clock begining
  $('#homeTimeContainer').html(playingStatus.time);

  if (oldPlayingStatus.status != playingStatus.status) {
    //status changed redraw page
    if (playingStatus.status == 'idle') {
      populateTableIdle();
    } else if (playingStatus.status == 'playing') {
      populateTablePlaying();

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


function populateTableIdle(status, query) {

  $.ajax({
    url: '/xScheduleQuery?Query=GetPlayLists',
    dataType: "json",
    indexValue: status,
    success: function(response) {
      populatePlaylistBox('playlists', 'Avalible Playlists');

      for (var i = 0; i < response.playlists.length; i++) {


        var idle =
          `<tr id='table` + response.playlists[i].id + `'>
                <td class="col-md-8">` + response.playlists[i].name + `</td>
                <td class="col-md-3">` + response.playlists[i].nextscheduled + `</td>
                <td class="col-md-1">` + response.playlists[i].length.split(".")[0] + `</td>
                <td class="col-md-2">
                  <button type="button" onclick="runCommand('Play specified playlist', '` + response.playlists[i].name + `')" class="btn btn-info btn-xs" name="button">Play</button>
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

      populatePlaylistBox('steps', playingStatus.playlist + ': ');

      for (var i = 0; i < response.steps.length; i++) {
        var playing =
          `<tr id='table` + response.steps[i].id + `'>
              <td class="col-md-8">` + response.steps[i].name + `</td>
              <td class="col-md-1">` + response.steps[i].length.split(".")[0] + `</td>
              <td class="col-md-2">
                  <button type="button" onclick="runCommand('Play playlist starting at step', '` + playingStatus.playlist + `,` + response.steps[i].name + `')" class="btn btn-info btn-xs " name="button" title="Jump to this step">Play</button>
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


function dashboardLoadStatus() {

  if (playingStatus.status == "idle") {
    //Populate Idle Page
    var notPlaying = '<p><center>Show Not Running!</center></p>';
    $('#currentStep').html(notPlaying);
    $("#controlButtonContainer").html("");


  } else {
    //playing

    var toggleButtons =
      `<div class="span6 text-center">
    <button onclick="runCommand('Prior step in current playlist')" class="btn btn-default glyphicon glyphicon-backward" title='Back' type="button"></button>
    <button onclick="runCommand('Pause')" class="btn btn-default glyphicon glyphicon-play" title='Play' type="button"></button>
    <button onclick="runCommand('Pause')" class="btn btn-default glyphicon glyphicon-pause" title='Pause' type="button"></button>
    <button onclick="runCommand('Stop all now')" class="btn btn-default glyphicon glyphicon-stop" title='Stop All' ype="button"></button>
    <button onclick="runCommand('Next step in current playlist')" class="btn btn-default glyphicon glyphicon-forward" title='Next' type="button"></button>
    </div>`;
    //CLEAN UP!
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
    $("#controlButtonContainer").html(toggleButtons);
    //Set bar
    // $("#currentPlaylistLoadingBar").css("width", findPercent(
    //   playingStatus
    //   .length.split(".")[0], playingStatus.left.split(".")[0]));
    //updatePlaylistSteps(playingStatus.playlist, playingStatus.step);
  }
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
