// Put all onload AJAX calls here, and event listeners
var tableHeader = "<tr><th>File name (click to download)</th><th>Version</th><th>Creator</th><th>Number of Waypoints</th><th>Number of Routes</th><th>Number of Tracks</th></tr>";
var gpxPanelHeader = "<tr><th>Component</th><th>Name</th><th>Number of points</th><th>Length</th><th>Loop</th></tr>";
var dropDownDefault =  "<button class=\"dropbtn\">Select file</button><div class=\"dropdown-content\"></button>";
var currentButtonName;
var connectedToDB = 0;

//*STUFF TO BE DONE ON PAGE LOAD
jQuery(document).ready(function() {
    //*UPDATE FILE LOG PANEL
    jQuery.ajax({
        type: 'get',
        dataType: 'json',
        url: '/updateFileLog',
        data: {},
        success: function (data) {
            var logTable = tableHeader;
            var vpDropBtn = dropDownDefault;
            var viewPanelButton = ""; 
            var fileName;
            var version;
            var creator;
            var numW;
            var numR;
            var numT;
            var tableFiles = data.files;

            //*FILE FILE LOG PANEL
            for(var i = 0; i < tableFiles.length; i++) {
                console.log("test loop\n");
                fileName = tableFiles[i].name;
                version = tableFiles[i].version;
                creator = tableFiles[i].creator;
                numW = tableFiles[i].numWaypoints;
                numR = tableFiles[i].numRoutes;
                numT = tableFiles[i].numTracks;
                //\"" + fileName + "\"
                //var newButton = "<a href=\"simple.gpx\" onclick=\"chooseView(this)\">"+ fileName + "</a>";
                // vpDropBtn = "<button class=\"dropbtn\">" + fileName + "</button><div class=\"dropdown-content\"></button>";

                var newButton = "<button onclick=\"chooseView(\'" + fileName +"\')\">"+ fileName + "</button>";
                var newRow = "<tr>";                //new row
                var col1 = "<td><a href=\"";        //name & link
                var col2 = "<td>";                  //version
                var col3 = "<td>";                  //creator
                var col4 = "<td>";                  //num waypoints
                var col5 = "<td>";                  //num routes
                var col6 = "<td>";                  //num tracks

                col1 = col1 + fileName + "\" download>" + fileName + "</a></td>";
                col2 = col2 + version + "</td>";
                col3 = col3 + creator + "</td>";
                col4 = col4 + numW + "</td>";
                col5 = col5 + numR + "</td>";
                col6 = col6 + numT + "</td>";

                newRow = newRow + col1 + col2 + col3 + col4 + col5 + col6 + "</tr>";
                console.log(newButton);
                viewPanelButton = viewPanelButton + newButton + "</div></div>";
                logTable = logTable + newRow;
            }

            if(tableFiles.length == 0) {
                logTable = logTable + "<tr><td>No files</td><td></td><td></td><td></td><td></td><td></td></tr>";
            }

            jQuery('#fileLogData').html(logTable);
            jQuery('#vp-dropdown').html(viewPanelButton);

            //jQuery('#gpxDropButton').html(viewPanelButton);
            console.log(data); 
        },
        fail: function(error) {
            // Non-200 return, do something with error
            $('#fileLogData').html("On page load, received error from server");
            console.log(error); 
        }
    });
    $('#pathForm').submit(function(e){
        e.preventDefault();
    });
    $('#gpxForm').submit(function(e){
        e.preventDefault();
        //location.reload();
    });
    $('#loginForm').submit(function(e){
        e.preventDefault();
        //location.reload();
    });
    $('#storeFiles').submit(function(e){
        e.preventDefault();
        //location.reload();
    });
});

//*UPDATE GPX VIEWPANEL
function chooseView(fileName) {

    console.log("\nchooseView fileName: " + fileName);

    jQuery.ajax({
        type: 'get',
        dataType: 'json',
        url: '/updateGPXViewPanel/' + fileName,
        data: {},
        success: function (data) {
            if(data.data != null) {
                console.log(data.data);
            }
            let rteListJSON = data.data.routes;
            let trkListJSON = data.data.tracks;

            var viewTable = "";
            var componentName;
            var name;
            var numPoints;
            var length;
            var loop;
            var vpBtn = vpDropBtn = "<button class=\"dropbtn\">" + fileName + "</button><div class=\"dropdown-content\"></button>";

            var addRouteBox = "<h6>Add Route to Selected File</h6>";
            addRouteBox = addRouteBox + "<form ref='routeForm'id='routeForm'><div class=\"form-group\">";
            addRouteBox = addRouteBox + "<input type=\"text\" class=\"route-form\" id=\"routeBox1\" value=\"\" placeholder=\"Enter Route name here\"></div>";
            addRouteBox = addRouteBox + "<div class=\"route-form\"><input onclick=\"addRouteToFile('"+ fileName +"')\" type='submit' class=\"btn btn-secondary\"></div></form>";
            viewTable = viewTable + gpxPanelHeader;
            //*GET ROUTE LIST INFO
            for(var i = 0; i < rteListJSON.length; i++) {

                componentName = "Route " + (i + 1);
                name = rteListJSON[i].name;
                numPoints = rteListJSON[i].numPoints;
                length = rteListJSON[i].len;
                loop = rteListJSON[i].loop;

                var newRow = "<tr>";                //new row
                var col1 = "<td><label><input name=\"r1\" type=\"radio\" onclick=\"viewPanelRadio('" + fileName + "','Route'," + i +")\">";                  //componentName
                var col2 = "<td>";                  //name
                var col3 = "<td>";                  //numPoints
                var col4 = "<td>";                  //length
                var col5 = "<td>";                  //loop

                col1 = col1 + componentName + "</td>";
                col2 = col2 + name + "</td>";
                col3 = col3 + numPoints + "</td>";
                col4 = col4 + length + "</td>";
                col5 = col5 + loop + "</td>";

                newRow = newRow + col1 + col2 + col3 + col4 + col5 + "</tr>";

                viewTable = viewTable + newRow;
            }

            //*GET TRACK LIST INFO
            for(var i = 0; i < trkListJSON.length; i++) {

                componentName = "Track " + (i + 1);
                name = trkListJSON[i].name;
                numPoints = trkListJSON[i].numPoints;
                length = trkListJSON[i].len;
                loop = trkListJSON[i].loop;

                var newRow = "<tr>";                //new row
                //var col1 = "<td>";
                var col1 = "<td><label><input name=\"r1\" type=\"radio\" onclick=\"viewPanelRadio('" + fileName + "','Track'," + i +")\">";                  //componentName 
                var col2 = "<td>";                  //name
                var col3 = "<td>";                  //numPoints
                var col4 = "<td>";                  //length
                var col5 = "<td>";                  //loop

                col1 = col1 + componentName + "</td>";
                col2 = col2 + name + "</td>";
                col3 = col3 + numPoints + "</td>";
                col4 = col4 + length + "</td>";
                col5 = col5 + loop + "</td>";

                newRow = newRow + col1 + col2 + col3 + col4 + col5 + "</tr>";

                viewTable = viewTable + newRow;
            }
            jQuery('#vpbtn').html(vpBtn);
            jQuery('#GPXViewData').html(viewTable);
            jQuery('#addRoute_box').html(addRouteBox);
            $('#routeForm').submit(function(e){
                e.preventDefault();
            });
            jQuery('#other_data').html("");
            jQuery('#rename_box').html("");

        },
        fail: function(error) {
            // Non-200 return, do something with error
            $('#fileLogData').html("On page load, received error from server");
            console.log(error); 
        }
    });
}

function viewPanelRadio(fileName, dataName, dataIndex) {

    console.log("viewPanelRadio fileName: " + fileName);
    console.log("viewPanelRadio dataName: " + dataName);
    console.log("viewPanelRadio dataIndex: " + dataIndex);


    var renameBox = "<h6>Rename " + dataName + " " + (dataIndex + 1) +" </h6>";
    renameBox = renameBox + "<form ref='renForm'id='renForm'><div class=\"form-group\">";
    renameBox = renameBox + "<input type=\"text\" class=\"ren-form\" id=\"renBox\" value=\"\" placeholder=\"Enter name here\"></div>";
    renameBox = renameBox + "<div class=\"ren-form\"><input onclick=\"renameRadio('"+ fileName +"','" + dataName + "'," + dataIndex + ")\" type='submit' class=\"btn btn-secondary\"></div></form>";

    var addWaypBox = "<h6>Add Waypoint to " + dataName + " " + (dataIndex + 1) + "</h6>";
    addWaypBox = addWaypBox + "<form ref='waypForm'id='waypForm'><div class=\"form-group\">";
    addWaypBox = addWaypBox + "<input type=\"text\" class=\"wayp-form\" id=\"waypBox1\" value=\"\" placeholder=\"Enter name here\"></div>";
    addWaypBox = addWaypBox + "<input type=\"text\" class=\"wayp-form\" id=\"waypBox2\" value=\"\" placeholder=\"Enter latitude here\"></div>";
    addWaypBox = addWaypBox + "<input type=\"text\" class=\"wayp-form\" id=\"waypBox3\" value=\"\" placeholder=\"Enter longitude here\"></div>";
    addWaypBox = addWaypBox + "<div class=\"wayp-form\"><input onclick=\"addWaypointRouteTrack('"+ fileName +"','" + dataName + "'," + dataIndex + ")\" type='submit' class=\"btn btn-secondary\"></div></form>";
    
    var viewTable = viewTable + gpxPanelHeader;
    var otherData = "No other data!";

    jQuery.ajax({
        type: 'get',
        dataType: 'json',
        url: '/updateOtherData/' + fileName,
        data: {
            otherName: dataName
            ,dIndex: dataIndex
        },
        success: function (data) {
            if(data.data != null) {
                console.log(data.data);
            }

            if(data.data != null) {
                otherData = data.data;
            }
            
            //*GET ROUTE LIST INFO
            jQuery('#other_data').html(otherData);

            //*ADD RENAME BOX
            jQuery('#rename_box').html(renameBox);
            $('#renForm').submit(function(e){
                e.preventDefault();
                chooseView(fileName);
            });

            //*ADD WAYPOINT BOX
            jQuery('#addWayp_box').html(addWaypBox);
            $('#waypForm').submit(function(e){
                e.preventDefault();
                chooseView(fileName);
            });

        },
        fail: function(error) {
            // Non-200 return, do something with error
            $('#fileLogData').html("On page load, received error from server");
            console.log(error); 
        }
    });
}

function renameRadio(fileName, dataName, dataIndex) {

    console.log("renameRadio fileName: " + fileName);
    console.log("renameRadio dataName: " + dataName);
    console.log("renameRadio dataIndex: " + dataIndex);
    var nVal = document.getElementById("renBox").value;
    console.log("renameRadio value: " + nVal);

    
    //var renameBox = 

    jQuery.ajax({
        type: 'post',
        dataType: 'json',
        url: '/renameRouteTrack/' + fileName,
        data: {
            dName: dataName
            ,dIndex: dataIndex
            ,nameVal: nVal
        },
        success: function (data) {
            if(data.data == 0) {
                console.log(data.data);
                alert("Failed to rename " + dataName + " " + (dataIndex + 1) + "!" + "\nNo name entered.");
            }

            if(data.data != null) {
                otherData = data.data;
            }
            
            //*GET ROUTE LIST INFO
            
            //jQuery('#other_data').html(otherData);
        },
        fail: function(error) {
            // Non-200 return, do something with error
            $('#fileLogData').html("On page load, received error from server");
            console.log(error); 
        }
    });
}

function addWaypointRouteTrack(fileName, dataName, dataIndex) {
    
    console.log("addWaypointRouteTrack:");
    console.log(fileName + " " + dataName +" " + dataIndex);

    var waypName = document.getElementById("waypBox1").value;
    var waypLat = document.getElementById("waypBox2").value;
    var waypLon = document.getElementById("waypBox3").value;
    console.log(waypName + " " + waypLat +" " + waypLon);

    jQuery.ajax({
        type: 'post',
        dataType: 'json',
        url: '/addWaypToRT/',
        data: {
            fName: fileName
            ,dName: dataName
            ,dIndex: dataIndex
            ,wName: waypName
            ,wLat: waypLat
            ,wLon: waypLon
        },
        success: function (data) {
            if(data.data == 0) {
                console.log(data.data);
                alert("Failed to add waypoint to " + fileName + "!");
            }
            else {
                if(connectedToDB == 1) {
                    storeFilesDB();
                }
                chooseView(fileName);
            }
        },
        fail: function(error) {
            // Non-200 return, do something with error
            $('#blah').html("Failed addWaypointRouteTrack");
            console.log(error); 
        }
    });



   
}

function addRouteToFile(file) {

    console.log("addRoutToFile:");
    var rName = document.getElementById("routeBox1").value;
    var fName = file;
    console.log(rName);

    jQuery.ajax({
        type: 'post',
        dataType: 'json',
        url: '/addRouteToGPX/',
        data: {
            fileName: fName
            ,routeName: rName
        },
        success: function (data) {
            if(data.data == 0) {
                console.log(data.data);
                alert("Failed to add route to " + fName + "!");
            }
            else {
                chooseView(fName);
            }
            
            //*GET ROUTE LIST INFO
            
            //jQuery('#other_data').html(otherData);
        },
        fail: function(error) {
            // Non-200 return, do something with error
            $('#blah').html("Failed to add Route.");
            console.log(error); 
        }
    });
}

function createGPXFile() {

    console.log("createGPXFile:");
    var fName = document.getElementById("gpxBox1").value;
    var cName = document.getElementById("gpxBox2").value;

    console.log(fName + " " + cName);



    jQuery.ajax({
        type: 'post',
        dataType: 'json',
        url: '/createGPX/',
        data: {
            fileName: fName
            ,creatorName: cName
        },
        success: function (data) {
            if(data.data == 0) {
                console.log(data.data);
                alert("Failed to create " + fName + "!");
            }
            else {
                location.reload();
            }
            
            //*GET ROUTE LIST INFO
            
            //jQuery('#other_data').html(otherData);
        },
        fail: function(error) {
            // Non-200 return, do something with error
            $('#blah').html("Failed createGPXFile");
            console.log(error); 
        }
    });
}

function updatePaths() {

    var p1_lat = document.getElementById("pathBox1").value;
    var p1_lon = document.getElementById("pathBox2").value;
    var p2_lat = document.getElementById("pathBox3").value;
    var p2_lon = document.getElementById("pathBox4").value;
    var delta = document.getElementById("pathBox5").value;

    console.log("updatePaths: ");
    console.log("Point 1: " + p1_lat +" Point 2: " + p2_lat + " Delta: " + delta);

    var pathTable;

    jQuery.ajax({
        type: 'get',
        dataType: 'json',
        url: '/getPaths/',
        data: {
            point1_lat: p1_lat
            ,point1_lon: p1_lon
            ,point2_lat: p2_lat
            ,point2_lon: p2_lon
            ,d: delta
        },
        success: function (data) {
            if(data.data.length == 0) {
                console.log(data.data);
                alert("Failed to update path table!");
            }
            else {
                let rteListJSON = data.data;
                let trkListJSON = data.data;
                console.log("rteListJSON:");
                console.log(rteListJSON[0]["routes"]);
                
                var viewTable = "";
                var componentName;
                var name;
                var numPoints;
                var length;
                var loop;

                viewTable = viewTable + gpxPanelHeader;
                //*GET ROUTE LIST INFO
                if(rteListJSON != null) {
                    for(var i = 0; i < rteListJSON.length; i++) {

                        let currRteList = rteListJSON[i]["routes"];

                        for(var j = 0; j < currRteList.length; j++) {

                        
                            let currRte = currRteList[j];

                            componentName = "Route " + (i + 1);
                            name = currRte["name"];
                            numPoints = currRte["numPoints"];
                            length = currRte["len"];
                            loop = currRte["loop"];
            
                            var newRow = "<tr>";                //new row
                            var col1 = "<td>";                  //componentName
                            var col2 = "<td>";                  //name
                            var col3 = "<td>";                  //numPoints
                            var col4 = "<td>";                  //length
                            var col5 = "<td>";                  //loop
            
                            col1 = col1 + componentName + "</td>";
                            col2 = col2 + name + "</td>";
                            col3 = col3 + numPoints + "</td>";
                            col4 = col4 + length + "</td>";
                            col5 = col5 + loop + "</td>";
            
                            newRow = newRow + col1 + col2 + col3 + col4 + col5 + "</tr>";
            
                            viewTable = viewTable + newRow;
                        }
                    }
                }
                console.log("TEST\n");

                if(trkListJSON != null) {
                    //*GET TRACK LIST INFO
                    for(var i = 0; i < trkListJSON.length; i++) {

                        let currTrkList = trkListJSON[i]["tracks"];

                        console.log(currTrkList);

                        for(var j = 0; j < currTrkList.length; j++) {

                            let currTrk = currTrkList[j];

                            componentName = "Track " + (i + 1);
                            name = currTrk["name"];
                            numPoints = currTrk["numPoints"];
                            length = currTrk["len"];
                            loop = currTrk["loop"];

                            var newRow = "<tr>";                //new row
                            var col1 = "<td>";                  //componentName 
                            var col2 = "<td>";                  //name
                            var col3 = "<td>";                  //numPoints
                            var col4 = "<td>";                  //length
                            var col5 = "<td>";                  //loop

                            col1 = col1 + componentName + "</td>";
                            col2 = col2 + name + "</td>";
                            col3 = col3 + numPoints + "</td>";
                            col4 = col4 + length + "</td>";
                            col5 = col5 + loop + "</td>";

                            newRow = newRow + col1 + col2 + col3 + col4 + col5 + "</tr>";

                            viewTable = viewTable + newRow;
                        }
                    }
                }
                jQuery('#FindPathTable').html(viewTable);
            }
            
            //*GET ROUTE LIST INFO
            
            //jQuery('#other_data').html(otherData);
        },
        fail: function(error) {
            // Non-200 return, do something with error
            console.log("ERROR!");
            //$('#fileLogData').html("On page load, received error from server");
            console.log(error); 
        }
    });
}

//*LOGIN TO MYSQL DB
function loginDB() {

    console.log("loginDB:");
    var dbName = document.getElementById("loginBox1").value;
    var uName = document.getElementById("loginBox2").value;
    var psw = document.getElementById("loginBox3").value;

    console.log(dbName + " " + uName + " " + psw);

    jQuery.ajax({
        type: 'post',
        dataType: 'json',
        url: '/loginToDB/',
        data: {
            databaseName: dbName
            ,userName: uName
            ,password: psw
        },
        success: function (data) {

            if(data.data == 1) {
                connectedToDB = 1;
                $('#DB-UI').css('display', 'block');
            }
            else {
                connectedToDB = 0;
                alert("Failed to login to database!");
            }
            
            
            //*GET ROUTE LIST INFO
        },
        fail: function(error) {
            alert("Failed to login to database!");
            console.log(error); 
        }
    });
}

//*STORE FILES IN DB
function storeFilesDB() {

    console.log("storeFilesDB:");
    var connectVal = connectedToDB;
    jQuery.ajax({
        type: 'post',
        dataType: 'json',
        url: '/storeFilesToDB/',
        data: {
            connBool: connectVal
        },
        success: function (data) {

            if(data.data == 1) {
                console.log("Files stored successfully!");
                //*UPDATE DB TABLE
            }
            else {
                alert("Failed to store files!");
            }
            
            
            //*GET ROUTE LIST INFO
        },
        fail: function(error) {
            alert("Failed to store files!");
            console.log(error); 
        }
    });
}

//*CLEAR DB
function clearDataDB() {

    console.log("clearDataDB:");
    var connectVal = connectedToDB;
    jQuery.ajax({
        type: 'post',
        dataType: 'json',
        url: '/clearDataInDB/',
        data: {
            connBool: connectVal
        },
        success: function (data) {

            if(data.data == 1) {
                console.log("Database cleared successfully!");
                //*UPDATE DB TABLE
            }
            else {
                alert("Failed to clear database!");
            }
            
            
            //*GET ROUTE LIST INFO
        },
        fail: function(error) {
            alert("Failed to clear database!");
            console.log(error); 
        }
    });
}

//*SHOW DB STATUS
function displayDBStat() {

    console.log("displayDBStat:");
    var connectVal = connectedToDB;
    jQuery.ajax({
        type: 'post',
        dataType: 'json',
        url: '/displayInDBStat/',
        data: {
            connBool: connectVal
        },
        success: function (data) {

            if(data.data == []) {
                alert("Failed to display DB status!");
                
            }
            else {
                alert("Database has " + data.data[0] + " files, " + data.data[1] + " routes, and " + data.data[2] + " points");
                console.log("Displayed DB status!");
            }
        
        },
        fail: function(error) {
            alert("Failed to display DB status!");
            console.log(error); 
        }
    });
}