var xyzzyStatus = "0";
var xyzzyName;
// not started = "0"
// waiting for name = "1"
// playing = "2"
// gameover = "3"

function openXyzzy() {
  $('#xyzzy').modal('show');
  xyzzyStatus = "1";
  xyzzyDraw("step1");
  $('#xyzzySelectionDisable').disableSelection();
}

function startXyzzy(value) {

  var xyzzyName = $('#xyzzyName')[0].value;
  if (xyzzyName == '') {
    xyzzyName = "Donald Trump";
  }
  //var xyzzyMatrix = $('#xyzzyMatrix')[0].value;
  xyzzyCommand('initialise&p=Matrix,' + xyzzyName);
  xyzzyCommand('g');
  xyzzyStatusTimer();
  xyzzyDraw("step2");
  xyzzyStatus = "2";
  $("#xyzzyPlayerName").html(xyzzyName);

}

function xyzzyRestart() {
  xyzzyCommand('close');
  xyzzyDraw("step1");
  xyzzyStatus = "1";
}

function xyzzyReset() {
  xyzzyCommand('c');
  xyzzyCommand('g');
}

function xyzzyStatusTimer() {

  var timer = window.setInterval(function() {
    $.ajax({
      url: '/xyzzy?c=q',
      success: function(response) {
        console.log(response);

        if (response.result == 'running') {
          //update Score
          //update next peice
          $("#xyzzyNext").html(response.next);
          $("#xyzzyCurrentScore").html("Your Score: " + response.score);


        } else if (response.result == 'failed') {
          xyzzyStatus = "0";
          clearInterval(timer);
        } else if (response.result == 'gameover') {
          xyzzyStatus = "3";
          xyzzyDraw("step3");
          $("#xyzzyPlayerName").html(response.playername);
          $("#xyzzyScore").html("Your Score: " + response.score);
          console.log(response.score);
          $("#xyzzyHighScoreName").html(response.highscoreplayer);
          $("#xyzzyHighScore").html("High Score: " + response.highscore);
        }

        //stop if game closed
        if ($('body').attr('class') != "modal-open") {
          xyzzyCommand('close');
          xyzzyStatus = "0";
          clearInterval(timer);
        }
      }
    })

  }, 1000);

}

function xyzzyCommand(c) {
  $.ajax({
    url: '/xyzzy?c=' + c,
    success: function(response) {
      console.log("I win");
    }
  });
}

function xyzzyDraw(step) {
  var step1 =
    `<h2>Billy</h2>
  <h4>High Score: 420</h4>
  <br />
  <h2>Enter Name:</h2>
  <input id="xyzzyName" type="text" class="form-control">
  <button data-dismiss="modal" aria-label="Close" onclick="startXyzzy($('#xyzzyName').value)" class="btn btn-success">Play</button>`;

  var step2 =
    `<h2 id="xyzzyPlayerName"></h2>
  <h4 id="xyzzyCurrentScore">Your Score: 320</h4>
  <button id="xyzzyNext" class="btn btn-default tetrisL btn-xyzzy"></button>
  <br/><br/><br/>
  <button onclick="xyzzyCommand('s')" class="btn btn-default glyphicon glyphicon-arrow-up btn-xyzzy"></button>
  <br />
  <button onclick="xyzzyCommand('l')" class="btn btn-default glyphicon glyphicon-arrow-left btn-xyzzy"></button>
  <button onclick="xyzzyCommand('d')" class="btn btn-default glyphicon glyphicon-arrow-down btn-xyzzy"></button>
  <button onclick="xyzzyCommand('r')" class="btn btn-default glyphicon glyphicon-arrow-right btn-xyzzy"></button>
  <br /><br />
  <button onclick="xyzzyReset()" class="btn btn-default btn-warning">Reset</button>
  <button onclick="xyzzyCommand('c')" class="btn btn-default btn-danger">Quit</button>`;

  var step3 =
    `<h1>Game Over</h1>
    <h2 id="xyzzyPlayerName"></h2>
  <h4 id="xyzzyScore"></h4>
  <h2 id="xyzzyHighScoreName"></h2>
<h4 id="xyzzyHighScore"></h4>
  <br />
  <button onclick="xyzzyRestart()" class="btn btn-info">Restart</button>`;

  if (step == "step1") {
    $("#xyzzyContainer").html(step1);
  } else if (step == "step2") {
    $("#xyzzyContainer").html(step2);
  } else if (step == "step3") {
    $("#xyzzyContainer").html(step3);
  }

}
