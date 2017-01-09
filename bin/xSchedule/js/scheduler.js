$(window).load(function() {
  var currentPage = window.location.pathname.split("/").pop();
  $('#user-nav').load("core/usernav.html" );
  $('#sidebar').load("core/nav.html" );

  if(currentPage == "buttons.html"){
    if( /Android|webOS|iPhone|iPad|iPod|BlackBerry/i.test(navigator.userAgent) ) {
      $('div#user-nav').remove();
      $('div#sidebar').remove();
    }
  }
  if(currentPage == "index.html"){
      dashboardLoadButtons();
        window.setInterval(function(){
          dashboardPlaylistStatus();
        }, 1000);
  }
  if(currentPage == "settings.html"){
    loadSettings();
  }
  loadOptions();

  retrieveKey('webName', function(resp){
    title = "XLights Scheduler";
    if (resp != "") {
      title = resp;
    }
    $('#header h1').html(title);
  });
  retrieveKey('webColor', function(resp){
    value = "";
    if (resp != "") {
      value = resp;
    }
    updatePageColor(value);
  });
  if(currentPage == "buttons.html"){
    buttonsLoadButtons();
  }
});
function loadOptions(){
  retrieveKey('webName', function(resp){
    title = "XLights Scheduler";
    if (resp != "") {
      title = resp;
    }
    $('#header h1').html(title);
  });
  retrieveKey('webColor', function(resp){
    value = "";
    if (resp != "") {
      value = resp;
    }
    updatePageColor(value);
  });
}

function loadSettings(){
  retrieveKey('webName', function(resp){
    value = "xLights Scheduler";
    if (resp != "") {
      value = resp;
    }
    $("#webName")[0].value = value;
  });
  retrieveKey('webColor', function(resp){
    value = "";
    if (resp != "") {
      value = resp;
    }
    $("#webColor")[0].value = value;
    $("#webColorDisplay").css("background-color", value)
  });
  retrieveKey('webLoginRequire', function(resp){
    r = resp.split('\n');
    if (r[0] == "true") {
      $("#webLoginRequire").prop("checked", true).uniform('refresh');
    }else {
      $("#webLoginRequire").prop("checked", false).uniform('refresh');
    }
  });
  retrieveKey('webUsername', function(resp){
    value = "";
    if (resp != "") {
      value = resp;
    }
    $("#webUsername")[0].value = value;
  });
  retrieveKey('webPassword', function(resp){
    value = "";
    if (resp != "") {
      value = resp;
    }
    $("#webPassword")[0].value = value;
  });
}

// function scheduleCommand(command, param){
//   var ourQuery = new XMLHttpRequest();
//   var ourData = 'hi';
//   ourQuery.open('GET', '/xScheduleQuery?command='+command+'&Parameters='+param);
//   ourQuery.onload = function(){
//     ourData = JSON.parse(ourQuery.responseText);
//   };
//   ourQuery.send();
// };
   var value= {};
function dashboardLoadButtons(){
   $.ajax({
      url: '/xScheduleQuery?Query=GetButtons',
      dataType: "json",
      success: function(response){
        //do stuff
        for (var i = 0; i < response.buttons.length; i++) {
          var buttonName = response.buttons[i].replace(" ", "%20");

          var li = '<li><a href="#" onclick=buttonClick("'+buttonName+'")> <i class="icon-people"></i> '+response.buttons[i]+' </a></li>';
          $("#dashboardButtons ul").append(li);
        }
       //do stuff
      }
   });
}

function dashboardPlaylistStatus(){
  $.ajax({
     url: '/xScheduleQuery?Query=GetPlayingStatus',
     dataType: "json",
     success: function(response){
       if (response.status == "idle"){
          var notPlaying = '<p><center>Show Not Running!</center></p>';
          $('#currentStep').html(notPlaying);
          dashboardPlaylistSteps(false);
       }else{
          var currentPlaylist = `<span class="icon"><i class="icon-file"></i></span><h5>Playlist: `+response.playlist+`</h5>`
          var Playing = `
          <li>
            <div class="user-thumb"> <img width="40" height="40" alt="User" src="img/demo/av1.jpg"> </div>
            <div class="article-post">
              <p><a href="#">`+response.step+`</a> </p>

              <span class="user-info"> Remaining: `+response.left.split(".")[0]+` | Duration: `+response.length.split(".")[0]+`</span>
              </div>
              </div>
          </li>`;
          //$('#currentPlaylist').html(currentPlaylist);
          $("#currentStep").html(Playing);
          dashboardPlaylistSteps(true, response.playlist, response.step);
        }
    }
  });
}

function dashboardPlaylistSteps(status, playlist, currentStep){

  $.ajax({
     url: '/xScheduleQuery?Query=GetPlayListSteps&Parameters='+ playlist,
     dataType: "json",
     success: function(response){
       if (status == false){
         updatePlaylists();
       }else{
         updatePlaylistSteps(response, playlist, currentStep);
     }
      //do stuff
     }

  });

}
function buttonsLoadButtons(){
   $.ajax({
      url: '/xScheduleQuery?Query=GetButtons',
      dataType: "json",
      success: function(response){
        //do stuff
        for (var i = 0; i < response.buttons.length; i++) {
          var buttonName = response.buttons[i].replace(" ", "%20");

          var li = '<li><a href="#" onclick=buttonClick("'+buttonName+'")> <i class="icon-people"></i> '+response.buttons[i]+' </a></li>';
          $("#buttonsPageButtons ul").append(li);
        }
       //do stuff
      }
   });
}

function buttonClick(name){
  $.ajax({
     url: '/xScheduleCommand?Command=PressButton&Parameters='+name,
     success: function(response){
     }
  });
}

function updatePlaylistSteps(response, playlist, currentStep){
  $('#firstPlaylist').html("");
  $("#playlistStatus li").remove();
  for (var i = 0; i < response.steps.length; i++) {
    var activeclass="";
    if (currentStep == response.steps[i].name){
     activeclass = 'class="stepActive"';
   }
    var li =
    `
    <li `+activeclass+`>
      <div class="user-thumb"> <img width="40" height="40" alt="User" src="img/demo/av1.jpg"> </div>
      <div class="article-post">
        <p><a href="#" onclick="runCommand('Play playlist starting at step', '`+playlist+`,`+response.steps[i].name+`')">`+response.steps[i].name+`</a> </p>
        <span class="user-info"> Duration: `+response.steps[i].length.split(".")[0]+`</span>
      </div>
    </li>
    `
   var currentPlaylist = `<span class="icon"><i class="icon-file"></i></span><h5>Playlist: `+playlist+`</h5>`;
    $("#playlistStatus ul").append(li);
    $('#currentPlaylist').html(currentPlaylist);
  }
}

function updatePlaylists(){
  $.ajax({
     url: '/xScheduleQuery?Query=GetPlayLists&Parameters=',
     dataType: "json",
     success: function(response){
        $("#playlistStatus li").remove();
        for (var i = 0; i < response.playlists.length; i++) {
          var activeclass="";
          var notPlaying = `
          <li>
            <div class="user-thumb"> <img width="40" height="40" alt="User" src="img/demo/av1.jpg"> </div>
            <div class="article-post">
              <p><a href="#" onclick="runCommand('Play specified playlist', '`+response.playlists[i].name+`')">`+response.playlists[i].name+`</a> </p>
              <span class="user-info">Playlist Length : `+response.playlists[i].length.split(".")[0]+`</span>
            </div>
          </li>`;
         $("#playlistStatus ul").append(notPlaying);
        }

      }
});
}

function runCommand(name, param){
  $.ajax({
     url: '/xScheduleCommand?Command='+name+'&Parameters='+param,
     success: function(response){
     }
  });
}

function updatePageColor(color){
  document.querySelector("#sidebar").style.background = color;
  document.querySelector("body").style.background = color;
}

function storeKey(key, value) {
    // written by cp16net so blame him.
    $.ajax({
       type: "POST",
       url: '/xScheduleStash?Command=Store&Key='+key,
       data: value,
       success: function(response){
         console.log(response);
         //window.alert("WIN");
       },
       error: function(error){
         console.log("ERROR: " + error);
         window.alert("FAIL");
       }
    });
}

function retrieveKey(key, f) {
    // written by cp16net so blame him.
    $.ajax({
       type: "GET",
       url: '/xScheduleStash?Command=Retrieve&Key='+key,
       success: f,
       error: function(error){
         window.alert("ERROR: " + error);
       }
    });
}


function updateWebSettings(){

    // var inputs = $('#webSettings :input')
    // var dataArr = [];
    // for (var i = 0; i < inputs.length; i++) {
    //   dataArr.append({inputs[i].id:inputs[i].value})
    // }

  webName = document.getElementById('webName').value;
  webColor = document.getElementById('webColor').value;
  webLoginRequire = document.getElementById('webLoginRequire').checked.toString();
  webUsername = document.getElementById('webUsername').value;
  webPassword = document.getElementById('webPassword').value;

  var dataArr = [
    {'name': 'webName', 'value': webName},
    {'name': 'webColor', 'value': webColor},
    {'name': 'webLoginRequire', 'value': webLoginRequire},
    {'name': 'webUsername', 'value': webUsername},
    {'name': 'webPassword', 'value': webPassword}
  ];
  for (var i = 0; i < dataArr.length; i++) {
    console.log("saving data: " + dataArr[i] + "   name:"+dataArr[i].name+"   value:"+dataArr[i].value);
   storeKey(dataArr[i].name, dataArr[i].value);
  }
  $("#webSettings").submit(function(e) {
      e.preventDefault();
  });
  loadOptions();



  //for (var data in dataArr) {
    //console.log("saving data: " + data);
      //storeKey(data.name, data.value);
  //};

}
