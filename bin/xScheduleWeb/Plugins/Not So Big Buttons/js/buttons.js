$(document).ready(function() {
loadButtonsData()

if (getQueryVariable("args") == 'noheader'){
  $('#nav').css('display', 'none');
  $('#header').css('display', 'none');
  $('#footer').css('display', 'none');
}
})
function loadButtonsData(){
  if (socket.readyState <= '1') {
    socket.send('{"Type":"Query","Query":"GetButtons", "Reference":"populateButtons"}');
  }else{
    $.ajax({
      url: '/xScheduleQueryQuery=' + command,
      success: function(response) {
        if (response.result == 'ok')
          populateButtons(response);
        if (response.result == 'failed')
          notification('Failed: ' + response.message, 'danger', '0');
      },
      error: function(response) {
        notification(response.result + ': ' + response.message, 'danger', '1');
      }
    });
  }
}
function populateButtons(response){
  console.log(response);
  for (var i = 0; i < response.buttons.length; i++) {

    if (response.buttons[i].color == "default"){
      var color = "btn-default";
    }else if (response.buttons[i].color == "blue"){
      var color = "btn-primary";
    }else if (response.buttons[i].color == "green"){
      var color = "btn-success";
    }else if (response.buttons[i].color == "cyan"){
      var color = "btn-info";
    }else if (response.buttons[i].color == "orange"){
      var color = "btn-warning";
    }else if (response.buttons[i].color == "red"){
      var color = "btn-danger";
    }

    var button = `<button onclick="runCommand('PressButton', 'id:`+response.buttons[i].id+`')" class="btn `+color+` notSoBigButtons"><div class="buttonLabel">`+response.buttons[i].label+`</div></button>`;
    $('#buttonContainer').append(button);
  }
}
