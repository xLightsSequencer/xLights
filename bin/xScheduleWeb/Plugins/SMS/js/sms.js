$(document).ready(function() {
	loadSMSData();
	setInterval(function(){loadSMSData();}, 5000);

	if (getQueryVariable("args") == 'noheader'){
	  $('#nav').css('display', 'none');
	  $('#header').css('display', 'none');
	  $('#footer').css('display', 'none');
	}
})

function loadSMSData(){
  if (socket.readyState <= '1') {
    socket.send('{"Type":"xSMSDaemon","Command":"GetSMSQueue", "Reference":"populateSMS"}');
  }else{
    $.ajax({
      url: '/xSMSDaemon?Command=' + command,
      success: function(response) {
        if (response.result == 'ok')
          populateSMS(response);
        if (response.result == 'failed')
          notification('Failed: ' + response.message, 'danger', '0');
      },
      error: function(response) {
        notification(response.result + ': ' + response.message, 'danger', '1');
      }
    });
  }
}

function populateSMS(response){
  //console.log(response);
  $("#smstable tr.smsrow").remove();
  for (var i = 0; i < response.messages.length; i++) {
	var disp = "";
	if (response.messages[i].displayed == "true")
	{
		disp = "background-color: #FFFF80;";
	}
    var tr = "<tr class='smsrow' style='border-bottom: 1px solid black;"+disp+"'><td width='18%'>" + response.messages[i].timestamp + "</td>" +
		"<td width='30%'>" + response.messages[i].status + "</td>" +
		"<td>" + response.messages[i].message + "</td></tr>";
	//console.log(tr);
    $('#smstable').append(tr);
  }
}
