var pausedSMS = false;

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
	
  if (pausedSMS) return;
	
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

function AMR(response)
{
  console.log(response);	
  loadSMSData();
}

function AllowMessage(message, allow, id)
{
  var all = "y";
  if (!allow) all = "n";
  if (socket.readyState <= '1') {
	var msg = '{"Type":"xSMSDaemon","Command":"ModerateSMS", "Reference":"AMR", "Data":"'+all+',' + message.substring(0, 40)+'"}';
	//console.log(msg);
	//console.log(msg.length);
    socket.send(msg);
	
	if (pausedSMS)
	{
		var but = "#mod" + id;
		$(but).attr('class', 'btn-filled-amber');
		if (allow)
		{
			$(but).text('Suppress');
			$(but).attr('onclick', 'AllowMessage("'+message+'", false, '+id+')');
		}
		else
		{
			$(but).text('Allow');
			$(but).attr('onclick', 'AllowMessage("'+message+'", true, '+id+')');
		}
	}
  }else{
    $.ajax({
      url: '/xSMSDaemon?Command=' + command,
      success: function(response) {
        if (response.result == 'ok')
		  LoadSMSData();
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
    var tr = "<tr class='smsrow' style='border-bottom: 1px solid black;"+disp+"'>'" +
	         "<td style='width: 18%; padding-top: 1em; padding-bottom: 1em;'>" + response.messages[i].timestamp + "</td>" +
		     "<td style='width: 25%;'>" + response.messages[i].status + "</td>";
	if (response.messages[i].moderatedok != "disabled")
	{
		if (response.messages[i].moderatedok == "true")
		{
			tr += '<td style="width: 20%;"><button id="mod'+i+'" class="btn-filled-red" onclick="AllowMessage(\''+response.messages[i].message+'\', false, '+i+')">Suppress</button></td>';
		}
		else
		{
			tr += '<td style="width: 20%;"><button id="mod'+i+'" class="btn-filled-green" onclick="AllowMessage(\''+response.messages[i].message+'\', true, '+i+')">Allow</button></td>';
		}
	}
	tr += "<td>" + response.messages[i].message + "</td>"  

	tr += "</tr style='width: 37%;'>";
	//console.log(tr);
    $('#smstable').append(tr);
  }
}

function pauseSMS(pause)
{
	if (pause)
	{
		$('#pause_button').attr('onclick', 'pauseSMS(false)');
		$('#pause_button').text('Paused');
		$('#pause_button').attr('class', 'btn-filled-red');
		pausedSMS = true;
	}
	else
	{
		$('#pause_button').attr('onclick', 'pauseSMS(true)');
		$('#pause_button').text('Updating');
		$('#pause_button').attr('class', 'btn-filled-green');
		pausedSMS = false;
		loadSMSData();
	}
}
