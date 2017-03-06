$(document).ready(function() {
  if (getQueryVariable("args") == false) {
    playlistsLoadPlaylists();
  } else {
    playlistsLoadPlaylistsSteps(getQueryVariable("args"));
  }
  populateSideBar();

});

function populateSideBar() {
  if (uiSettings != undefined) {
    if (uiSettings.playlists[0] == true) {
      $('#sideBar1').load('inc/sidebarNav.html');
    }
    if (uiSettings.playlists[1] == true) {
      $('#sideBar2').load('inc/sidebarPlayerStatus.html');
    }
  }
}

function playlistsLoadPlaylists() {
  $.ajax({
    url: '/xScheduleQuery?Query=GetPlayLists&Parameters=',
    dataType: "json",
    success: function(response) {

      $('#currentPlaylist').html("Avalible Playlists");

      $("#playlistStatus tr").remove();
      for (var i = 0; i < response.playlists.length; i++) {
        var activeclass = "";
        var notPlaying =
          `<tr>
            <td>` + response.playlists[i].name +
          `</td>
            <td>` + response.playlists[i].length.split(
            ".")[0] +
          `</td>
            <td>
              <button type="button" onclick="runCommand('Play specified playlist', '` + response.playlists[i].name + `')" class="btn btn-info btn-xs" name="button">Play</button>
              <button type="button" onclick="updatePage('page', 'playlists','` + response.playlists[i].name + `')" class="btn btn-default btn-xs" name="button">View</button>
            </td>
          </tr>`;

        $("#playlist").append(notPlaying);
      }
    }
  });
}

function playlistsLoadPlaylistsSteps(playlist, currentStep) {
  $.ajax({
    url: '/xScheduleQuery?Query=GetPlayListSteps&Parameters=' + playlist,
    dataType: "json",
    success: function(response) {
      var controls = `
      <span style="float:right;">
      <button type="button" onclick="runCommand('Play specified playlist', '` + playlist + `'); updatePage('page', 'home');" class="btn btn-info btn-xs" name="button" title="Play this playlist now">Play Playlist</button>
      </span>`;

      $('#currentPlaylist').html("Playlist: " + playlist + controls);

      $("#playlistStatus tr").remove();
      for (var i = 0; i < response.steps.length; i++) {
        var activeclass = "";
        var notPlaying =
          `<tr>
            <td>` + response.steps[i].name +
          `</td>
            <td>` + response.steps[i].length.split(
            ".")[0] +
          `</td>
            <td>
              <button type="button" onclick="runCommand('Play playlist starting at step', '` + playlist + `,` + response.steps[i].name + `')" cl)" class="btn btn-info btn-xs" name="button" title="Play playlist starting at this step">Play</button>
              <button type="button" onclick="runCommand('Enqueue playlist step', '` + playlist + `,` + response.steps[i].name + `')" class="btn btn-default btn-xs glyphicon glyphicon-plus" name="button" title="Queue song"></button>
            </td>
          </tr>`;

        $("#playlist").append(notPlaying);
      }
    }
  });
}
