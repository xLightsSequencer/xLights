$(document).ready(function() {
  if (getQueryVariable("args") == false) {
    playlistsLoadPlaylists();
  } else {
    playlistsLoadPlaylistsSteps(getQueryVariable("args"));
  }
  populateSideBar();

});

function playlistsLoadPlaylists() {
  $.ajax({
    url: '/xScheduleQuery?Query=GetPlayLists&Parameters=',
    dataType: "json",
    success: function(response) {

      $('#currentPlaylist').html("Available Playlists");

      $("#playlistStatus tr").remove();
      for (var i = 0; i < response.playlists.length; i++) {
        var activeclass = "";
        var notPlaying =
          `<tr>
            <td>` + response.playlists[i].name +
          `</td>
            <td class="col-md-2">` + response.playlists[i].length.split(
            ".")[0] +
          `</td>
            <td class="col-md-2">
              <button type="button" onclick="runCommand('Play specified playlist', 'id:` + response.playlists[i].id + `')" class="btn btn-info btn-xs" name="button">Play</button>
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
      <button type="button" onclick="updatePage('page','playlists')" class="btn btn-info btn-xs" name="button" title="Back to All Playlists">Back</button>
      </span>`;

      $('#currentPlaylist').html("Playlist: " + playlist + controls);

      $("#playlistStatus tr").remove();
      for (var i = 0; i < response.steps.length; i++) {
        var activeclass = "";
        var notPlaying =
          `<tr>
            <td>` + response.steps[i].name +
          `</td>
            <td class="col-md-2">` + response.steps[i].length.split(
            ".")[0] +
          `</td>
            <td class="col-md-2">
              <button type="button" onclick="runCommand('Play playlist starting at step', '` + playlist + `,id:` + response.steps[i].id + `'); updatePage('page', 'home');" class="btn btn-info btn-xs" name="button" title="Play playlist starting at this step">Play</button>
              <!--<button type="button" onclick="runCommand('Enqueue playlist step', '` + playlist + `,'id:` + response.steps[i].id + `')" class="btn btn-default btn-xs glyphicon glyphicon-plus" name="button" title="Queue song"></button>-->
            </td>
          </tr>`;

        $("#playlist").append(notPlaying);
      }
      $('#playlist').dataTable({
        "ordering": false,
        "searching": false,
      });
    }
  });
}
