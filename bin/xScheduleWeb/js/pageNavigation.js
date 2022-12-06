$(document).ready(function() {
  //populate current page
  updateCurrentPage();

});

window.addEventListener('popstate', function(event) {
  console.log("navagated");
  updateCurrentPage();
})
var currentPage;

function updateCurrentPage() {
  var currentPageArgs = getQueryVariable("args");
  if (getQueryVariable("args") == false) {
    var currentPageArgs = "";
  }
  if (getQueryVariable("page") != false) {
    currentPage = getQueryVariable("page");
    updatePage('page', currentPage, currentPageArgs);
  } else if (getQueryVariable("plugin") != false) {
    currentPage = getQueryVariable("plugin");
    updatePage('plugin', currentPage, currentPageArgs);
  } else {
    currentPage = 'home';
    updatePage('page', 'home');
  }
}


function updatePage(type, pageName, args) {
  //defocus nav
  $('#home').attr('class', "");
  $('#playlists').attr('class', "dropdown");
  $('#plugins').attr('class', "dropdown");
  $('#settings').attr('class', "");
  currentPage = pageName;

  if (/Android|webOS|iPhone|iPad|iPod|BlackBerry/i.test(navigator.userAgent)) {
    $('#navbar').collapse('hide');
  }

  if (type == "page") {
    //populate new page
    $('#pageContent').load('pages/' + pageName + '.html');
    $("#" + pageName).attr('class', "active");
    if (args != undefined) {
      window.history.pushState('page2', pageName, 'index.html?page=' +
        pageName +
        `&args=` + args);
    } else {
      window.history.pushState('page2', pageName, 'index.html?page=' +
        pageName);
    }
  } else if (type == "plugin") {
	pageName = encodeURI(pageName);
    $('#pageContent').load('plugins/' + pageName + '/' + pageName + '.html');
    $('#navPlugins').attr('class', "dropdown active");
    if (args != "") {
      window.history.pushState('page2', pageName, 'index.html?plugin=' +
        pageName + `&args=` + args);
    } else {
      window.history.pushState('page2', pageName, 'index.html?plugin=' +
        pageName);
    }
  }
}

function getQueryVariable(variable) {
  var query = window.location.search.substring(1);
  var vars = query.split("&");
  for (var i = 0; i < vars.length; i++) {
    var pair = vars[i].split("=");
    if (pair[0] == variable) {
      return decodeURIComponent(pair[1]);
    }
  }
  return (false);
}
