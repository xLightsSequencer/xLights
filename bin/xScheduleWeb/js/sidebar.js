$(document).ready(function() {
  window.setInterval(function() {
    updateControllerStatus();
  }, 100);
});

function populateSideBar() {

        var sidebar1 = `
        <div class="list-group">
        <div class="list-group-item active main-color-bg"><span class="glyphicon glyphicon-cog" aria-hidden="true"></span> Navagation </div>
        <a href="" onclick="updatePage('page','home')" class="list-group-item"><span class="glyphicon glyphicon-list-alt" aria-hidden="true"></span>  Home </a>
        <a href="" onclick="updatePage('page','playlists')" class="list-group-item"><span class="glyphicon glyphicon-list-alt" aria-hidden="true"></span>  Playlists </a>
        <a href="" onclick="updatePage('page','settings')" class="list-group-item"><span class="glyphicon glyphicon-user" aria-hidden="true"></span>  Settings </a>
        </div>
        `;
        $('#sideBar1').html(sidebar1);

        var sidebar2 = `
        <div class="list-group">
        <div class="list-group-item active main-color-bg"><span class="glyphicon glyphicon glyphicon-sort" aria-hidden="true"></span> Controller Status </div>
        <table class="table table-sm table-dark">
        <tbody id="controllerStatusTable">
        </tbody>
        </table>
        </div>
        `;
        $('#sideBar2').html(sidebar2);


  }


//update controller status

var oldControllerStatus = '';

function updateControllerStatus(oldControllerStatus){

  if (oldControllerStatus != playingStatus.pingstatus){
    //clear old data
    $('#controllerStatusTable').html("");

    //update new status
    for (var i = 0; i < playingStatus.pingstatus.length; i++) {
      var statusColor;
      if (playingStatus.pingstatus[i].result == "Unavailable"){ statusColor = '#ff9800'; textColor = '#000'; }
      if (playingStatus.pingstatus[i].result == "Failed"){ statusColor = '#ff0000'; textColor = '#000'; }
      if (playingStatus.pingstatus[i].result == "Ok"){ statusColor = '#00ff0a'; textColor = '#000'; }
      if (playingStatus.pingstatus[i].result == "Unknown"){ statusColor = '#ffd600'; textColor = '#000'; }
      var controller =
      `
      <tr class="bg-primary" style="background-color: `+statusColor+`; color:`+textColor+`;">
        <th data-original-title="222" data-container="body" data-toggle="tooltip" data-placement="bottom" class="matrisHeader">`+playingStatus.pingstatus[i].controller+` - `+playingStatus.pingstatus[i].result+`</th>
      </tr>
      `;
      $('#controllerStatusTable').append(controller);
      oldControllerStatus = playingStatus.pingstatus;
    }
  }

}
