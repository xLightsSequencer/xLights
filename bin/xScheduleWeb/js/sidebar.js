function populateSideBar() {
  if (uiSettings != undefined) {
    //disable custom sidebard for plugins
    if(getQueryVariable("plugin") == false){
      if (getPage(uiSettings, currentPage).values[0] == true) {
        var sidebar1 = `
        <div class="list-group">
        <div class="list-group-item active main-color-bg"><span class="glyphicon glyphicon-cog" aria-hidden="true"></span> Navagation </div>
        <a href="" onclick="updatePage('page','home')" class="list-group-item"><span class="glyphicon glyphicon-list-alt" aria-hidden="true"></span>  Home </a>
        <a href="" onclick="updatePage('page','playlists')" class="list-group-item"><span class="glyphicon glyphicon-list-alt" aria-hidden="true"></span>  Playlists </a>
        <a href="" onclick="updatePage('page','settings')" class="list-group-item"><span class="glyphicon glyphicon-user" aria-hidden="true"></span>  Settings </a>
      </div>
        `;
        $('#sideBar1').html(sidebar1);
      }
      if (getPage(uiSettings, currentPage).values[1] == true) {
        var sidebar2 = `
        <div class="panel panel-default">
          <div class="panel-heading main-color-bg">
            <h3 class="panel-title">Player Status</h3>
          </div>
          <div class="panel-body">
            <p>Coming Soon</p>
          </div>
        </div>
        `;
        $('#sideBar2').html(sidebar2);
      }
      //if no sidebar, remove it
      if (getPage(uiSettings, currentPage).values[0] == false && getPage(uiSettings, currentPage).values[1] == false) {
        $('#sidebarContainer').visibility = false;
        $('#mainContainer').css("width", "100%");
      }

    } else {
      sleep(100),
        function() {
          populateSideBar();
        }
    }
  }
}
