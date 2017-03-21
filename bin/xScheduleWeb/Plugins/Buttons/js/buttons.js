$(document).ready(function() {
loadButtonsData()
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
    var button = `<button onclick="runCommand('PressButton', 'id:`+response.buttons[i].id+`')" class="btn btn-default bigButtons"><wbr>`+response.buttons[i].label+`</wbr></button>`;
    $('#buttonContainer').append(button);
  }
}
