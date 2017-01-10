$(window).ready(function() {
  var currentPage = window.location.pathname.split("/").pop();
  //Populate User Navigation
  $('#user-nav').load("core/usernav.html" );
  //Populate Page Navigation
  $('#sidebar').load("core/nav.html" );

  //Current Page Settings
  if(currentPage == "buttons.html"){
    if( /Android|webOS|iPhone|iPad|iPod|BlackBerry/i.test(navigator.userAgent) ) {
      $('div#user-nav').remove();
      $('div#sidebar').remove();
    }
    buttonsLoadButtons();
  }
  if(currentPage == "index.html"){
        window.setInterval(function(){
          dashboardLoadStatus();
        }, 1000);
  }
  if(currentPage == "settings.html"){
    settingsLoadSettings();
  }
  //Page Settings^

  //Populate Page Name
  //Populate Page Color
  loadPageSettings();

});
//End onload

      // GLobal Populate

      function loadPageSettings(){
        //Populate Page Name
        //Populate Page Color
        retrieveKey('webName', function(resp){
          title = "XLights Scheduler";
          if (resp == "\n" || resp == null || resp == "") {
            storeKey("webName", title);
            //console.log("Default Name Saved "+ title);
          }else{
            title = resp;
            //console.log("Page Name Loaded "+ resp);
          }
          $('#header h1').html(title);
        });
        retrieveKey('webColor', function(resp){
          value = "#ff0000";
          if (resp == "\n" || resp == null || resp == "") {
            storeKey("webColor", value);
            //console.log("Default Color Saved: "+ value);
          }else{
            //console.log("Color Settings Loaded "+ resp);
            value = resp;
          }
          updatePageColor(value);
        });
      }

      function buttonsLoadButtons(){
         $.ajax({
            url: '/xScheduleQuery?Query=GetButtons',
            dataType: "json",
            success: function(response){
              for (var i = 0; i < response.buttons.length; i++) {
                var buttonName = response.buttons[i].replace(" ", "%20");

                var li = '<li><a href="#" onclick=buttonClick("'+buttonName+'")> <i class="icon-people"></i> '+response.buttons[i]+' </a></li>';
                $("#buttonsPageButtons ul").append(li);
              }
            }
         });
      }

            //Setting Page

            function settingsLoadSettings(){
              //Populate Settings Page
              retrieveKey('webName', function(resp){
                value = "xLights Scheduler";
                if (resp == "\n" || resp == null || resp == "") {
                  //console.log("Default Setting Loaded "+ value);
                }else{
                  value = resp;
                }
                $("#webName")[0].value = value;
                document.title = value;
              });
              retrieveKey('webColor', function(resp){
                value = "#ff0000";
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
            function settingsUpdateWebSettings(){

              var inputs = $('#webSettings :input')
              var dataArr = [];
              for (var i = 0; i < inputs.length; i++) {
                dataArr.push({'name': inputs[i].id, 'value': inputs[i].value });
                }
              for (var i = 0; i < dataArr.length; i++) {
               storeKey(dataArr[i].name, dataArr[i].value);
              }
              //prevent Page Reload
              $("#webSettings").submit(function(e) {
                  e.preventDefault();
              });
              //Reload Current Page Information
              loadPageSettings();
              settingsLoadSettings();


              // for (var data in dataArr) {
              //   console.log("saving data: " + data);
              //     storeKey(data.name, data.value);
              // };

            }

                    //Dashboard Page

                    function dashboardLoadStatus(){
                      $.ajax({
                         url: '/xScheduleQuery?Query=GetPlayingStatus',
                         dataType: "json",
                         success: function(response){
                           if (response.status == "idle"){
                              //Populate Idle Page
                              var notPlaying = '<p><center>Show Not Running!</center></p>';
                              $('#currentStep').html(notPlaying);
                              $("#currentPlaylistLoadingBarDiv").remove();
                              dashboardPopulatePlaylists();
                           }else{
                             if ($("#stepStatusBarContainer div").length ==0){
                               var html = `
                               <div id="currentPlaylistLoadingBarDiv" class="progress progress-striped active">
                                <div id="currentPlaylistLoadingBar" class="bar" style=""></div>
                               </div>`;

                               $("#stepStatusBarContainer").html(html);
                             }

                              var currentPlaylist = `<span class="icon"><i class="icon-file"></i></span><h5>Playlist: `+response.playlist+`</h5>`
                              var Playing = `
                                <li>
                                  <div class="user-thumb"> <img width="40" height="40" alt="User" src="img/demo/av1.jpg"> </div>
                                  <div class="article-post">
                                    <p><a onClick="runCommand()" href="#">`+response.step+`</a> </p>
                                    <span class="user-info"> Remaining: `+response.left.split(".")[0]+` | Duration: `+response.length.split(".")[0]+`</span>
                                    </div>
                                    </div>
                                </li>

                              `;
                              $("#currentStep").html(Playing);
                              $("#currentPlaylistLoadingBar").css("width", findPercent(response.length.split(".")[0], response.left.split(".")[0]));
                              dashboardUpdatePlaylistSteps(response.playlist, response.step);
                            }
                        }
                      });
                    }

                    function dashboardUpdatePlaylistSteps(playlist, currentStep){
                      $.ajax({
                         url: '/xScheduleQuery?Query=GetPlayListSteps&Parameters='+ playlist,
                         dataType: "json",
                         success: function(response){
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
                        });
                    }
                    function dashboardPopulatePlaylists(){
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
                             var currentPlaylist = `<span class="icon"><i class="icon-file"></i></span><h5>Avalible Playlists:</h5>`;
                             $('#currentPlaylist').html(currentPlaylist)
                            }
                          }
                      });
                    }


  //Utility

  function buttonClick(name){
    $.ajax({
       url: '/xScheduleCommand?Command=PressButton&Parameters='+name,
       success: function(response){
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
         },
         error: function(error){
           console.log("ERROR: " + error);
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
  function findPercent(length, left){
  var secLength = parseInt(length.split(":")[0]) * 60 + parseInt(length.split(":")[1]);
  var secLeft = parseInt(left.split(":")[0]) * 60 + parseInt(left.split(":")[1]);
  var percent = (secLength - secLeft) / secLength * 100;
  return Math.round(percent)+"%";
}
