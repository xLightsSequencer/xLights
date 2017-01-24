$(window).ready(function() {

      window.setInterval(function() {
        playlistsPopulatePlaylists();
        playlistsUpdatePlaylistSteps();
      }, 1000);

})
var selectedPlaylist = "";
function playlistsPopulatePlaylists() {
  $.ajax({
    url: '/xScheduleQuery?Query=GetPlayLists&Parameters=',
    dataType: "json",
    success: function(response) {
      $("#playlists li").remove();
      for (var i = 0; i < response.playlists.length; i++) {
        var activeclass = "";
        var li =
          `<li><div class="user-thumb"><img width="40" height="40" alt="User" src="img/demo/av1.jpg">
           </div><div class="article-post"><p><a href="#" onclick="selectPlaylist('`+response.playlists[i].name+`')">` + response.playlists[i].name +
          `</a></p>
          <span class="user-info">Playlist Length : ` +
          response.playlists[i].length.split(".")[0] +
          `</span></div></li>`;

        $("#playlists ul").append(li);
      }
    }
  });
}

function playlistsUpdatePlaylistSteps() {

  if (selectedPlaylist == ""){
    $("#playlistSteps li").remove();
    var noneSelected =
      `<li><p><center>Please select a Playlist</center></p></li>`;
      $("#playlistSteps ul").append(noneSelected);
  }else{
    $.ajax({
      url: '/xScheduleQuery?Query=GetPlayListSteps&Parameters=' + selectedPlaylist,
      dataType: "json",
      success: function(response) {
        $('#firstPlaylist').html("");
        $("#playlistSteps li").remove();
        for (var i = 0; i < response.steps.length; i++) {
          var activeclass = "";
          var li =
            `<li ` + activeclass +
            `><div class="user-thumb"> <img width="40" height="40" alt="User" src="img/demo/av1.jpg"> </div>
            <div class="article-post"><p><a href="#" onclick="runCommand('Play playlist starting at step', '` +
            selectedPlaylist + `,` + response.steps[i].name + `')">` +
            response.steps[i].name +
            `</a> </p><span class="user-info"> Duration: ` +
            response.steps[i].length.split(".")[0] +
            `</span>
                                  </div>
                                </li>
                                `
          var currentPlaylist =
            `<span class="icon"><i class="icon-file"></i></span><h5>Playlist: ` +
            selectedPlaylist + `</h5>`;
          $("#playlistSteps ul").append(li);
          $('#currentPlayliststeps').html(currentPlaylist);
        }
      }
    });
}
}

function selectPlaylist(selected){
  selectedPlaylist = selected;
  console.log(selected);
}
