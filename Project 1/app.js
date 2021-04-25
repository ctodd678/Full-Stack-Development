'use strict'

//*C library API
const ffi = require('ffi-napi');

//*TO GET FILES
const fileReader = require('fs');

//*MYSQL
const mysql = require('mysql2/promise');

var con;

var GPXParser = ffi.Library('./libgpxparser.so', {
  //*FUNCTIONS FROM GPX PARSER
  'GPXFileLogJSON': ['string', ['string']],
  'GPXViewPanelJSON': ['string', ['string']],
  'getOtherDataRadio': ['string', ['string','string','int']],
  'renameRouteTrack' : ['int', ['string','string','int','string']],
  'getPathJSON' : ['string', ['string','float','float','float','float','float']],
  'createGPXFile' : ['int', ['string','string']],
  'addRouteToFile' : ['int', ['string','string']],
  'addWaypointToFile' : ['int', ['string','string','int','string','double','double']],
  'rteWaypToJSON' : ['string', ['string','string']],
  'getRouteAtIndex' : ['string', ['string','int']]
});

var files;

// Express App (Routes)
const express = require("express");
const app     = express();
const path    = require("path");
const fileUpload = require('express-fileupload');

app.use(fileUpload());
app.use(express.static(path.join(__dirname+'/uploads')));

app.use(express.urlencoded({
  extended: true
}));

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

// Send HTML at root, do not change
app.get('/',function(req,res){
  res.sendFile(path.join(__dirname+'/public/index.html'));
});

// Send Style, do not change
app.get('/style.css',function(req,res){
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname+'/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js',function(req,res){
  fs.readFile(path.join(__dirname+'/public/index.js'), 'utf8', function(err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {compact: true, controlFlowFlattening: true});
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function(req, res) {
  if(!req.files) {
    return res.status(400).send('No files were uploaded.');
  }
 
  let uploadFile = req.files.uploadFile;
 
  // Use the mv() method to place the file somewhere on your server
  uploadFile.mv('uploads/' + uploadFile.name, function(err) {
    if(err) {
      return res.status(500).send(err);
    }

    res.redirect('/');
  });
});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req , res){
  fs.stat('uploads/' + req.params.name, function(err, stat) {
    if(err == null) {
      res.sendFile(path.join(__dirname+'/uploads/' + req.params.name));
    } else {
      console.log('Error in file downloading route: '+err);
      res.send('Unable to upload file!');
    }
  });
});

//******************** Your code goes here ******************** 

//*A4 FUNCTIONS

//*LOGIN TO DB
app.post('/loginToDB', async function(req, res, next) {

  var databaseN = req.body.databaseName;
  var userN = req.body.userName;
  var password = req.body.password;
  var success = 0;

  console.log("loginToDB: " + databaseN + " " + userN + " " + password);

  if (con && con.end) await con.end();

  // user = 'a4';
  // password = '089080';
  // databaseN = 'a4';
  //dursley.socs.uoguelph.ca
  try {
    con = await mysql.createConnection({
      host: 'dursley.socs.uoguelph.ca',
      user: userN,
      password: password,
      database: databaseN
    });

    await con.execute('CREATE TABLE IF NOT EXISTS FILE (gpx_id INT AUTO_INCREMENT, file_name VARCHAR(60) NOT NULL, ver DECIMAL(2,1) NOT NULL, creator VARCHAR(256) NOT NULL, PRIMARY KEY(gpx_id))');
    await con.execute('CREATE TABLE IF NOT EXISTS ROUTE (route_id INT AUTO_INCREMENT, route_name VARCHAR(256), route_len FLOAT(15,7) NOT NULL, gpx_id INT NOT NULL, PRIMARY KEY(route_id), FOREIGN KEY(gpx_id) REFERENCES FILE(gpx_id) ON DELETE CASCADE)');
    await con.execute('CREATE TABLE IF NOT EXISTS POINT (point_id INT AUTO_INCREMENT, point_index INT NOT NULL, latitude DECIMAL(11,7) NOT NULL, longitude DECIMAL(11,7), point_name VARCHAR(256), route_id INT NOT NULL, PRIMARY KEY(point_id), FOREIGN KEY(route_id) REFERENCES ROUTE(route_id) ON DELETE CASCADE)');

    success = 1;

  }
  catch(e) {
    console.log("Query error: " + e);
  } 

  res.send(
	  {
		data: success
	  }
	);

});


//*STORE FILES TO DB
app.post('/storeFilesToDB', async function(req, res, next) {

  const fileDir = './uploads/';
  var connBool = req.body.connBool;
  var success = 0;
  var fileN = 0;

  var fileLogJSON = [];

  console.log("storeFilesToDB: " + connBool);

  if (connBool == 1) {
    try {
      fs.readdir (fileDir, async (err, files) => {
        if(err)
          console.log(err);
        else {
          files.forEach(async file => {
    
            let tempFile = file;
            file = fileDir + file;  //add directory to file name
    
            if(file.includes(".gpx")) {  //only update .gpx files
              let gpxJSONStr = GPXParser.GPXFileLogJSON(file);
              if(gpxJSONStr != null) {
                let gpxJSON = JSON.parse(gpxJSONStr);
                gpxJSON["name"] = tempFile;
                fileLogJSON.push(gpxJSON);
  
                console.log("INSERTING TO FILE TABLE: " + gpxJSON["name"] + " " + gpxJSON["version"] + " " + gpxJSON["creator"]);
                await con.execute("DELETE FROM FILE WHERE file_name='" + gpxJSON["name"] + "';"); //overwrite files with same name
                await con.execute("INSERT INTO FILE (file_name, ver, creator) VALUES ('" + gpxJSON["name"] + "', '" + gpxJSON["version"] + "', '" + gpxJSON["creator"] + "')");
                
                let viewPanelStr = GPXParser.GPXViewPanelJSON(file);
                let viewPanelJSON = JSON.parse(viewPanelStr);
                
                let [row, field] = await con.execute('SELECT gpx_id FROM FILE WHERE file_name="' + gpxJSON["name"] +'"');
                console.log("GPX ID: " + row[0].gpx_id);
  
                for(var i = 0; i < viewPanelJSON["routes"].length; i++) {
                  let currRouteJSON = viewPanelJSON["routes"][i];
                  console.log("INSERTING TO ROUTE TABLE: " + currRouteJSON["name"] + " " + currRouteJSON["len"]);
                  await con.execute("DELETE FROM ROUTE WHERE route_name='" + currRouteJSON["name"] + "';"); //overwrite routes with same name
                  await con.execute("INSERT INTO ROUTE (route_name, route_len, gpx_id) VALUES ('" + currRouteJSON["name"] + "', '" + currRouteJSON["len"] + "', '" + row[0].gpx_id + "')");
  
                  let tmpFileStr = "./uploads/" + gpxJSON["name"];
                  let rteWaypStr = GPXParser.rteWaypToJSON(tmpFileStr, currRouteJSON["name"]);
                  console.log(rteWaypStr);
                  let rteWaypListJSON = JSON.parse(rteWaypStr);
                  console.log(rteWaypListJSON);
                  console.log(rteWaypListJSON["waypoints"].length);
  
                  let [row2, field2] = await con.execute('SELECT route_id FROM ROUTE WHERE route_name="' + currRouteJSON["name"] +'"');
                  for(var j = 0; j < rteWaypListJSON["waypoints"].length; j++) {
                    let currWaypJSON = rteWaypListJSON["waypoints"][j];
                    console.log("INSERTING TO POINT TABLE: " + currWaypJSON["name"] + " " + currWaypJSON["lat"] + " " + currWaypJSON["lon"]);
                    await con.execute("DELETE FROM POINT WHERE point_name='" + currWaypJSON["point_name"] + "';"); //overwrite routes with same name
                    await con.execute("INSERT INTO POINT (point_index, latitude, longitude, point_name, route_id) VALUES ('" + j + "', '" + currWaypJSON["lat"] + "', '" + currWaypJSON["lon"] + "', '" + currWaypJSON["name"] + "', '" + row2[0].route_id + "')");
                  }
                }
                fileN = fileN + 1;
              }
            }
          })
        }
      });
    }
    catch(e) {
      console.log("Query error: " + e);
    } 

    success = 1;
  }
  else {
    success = 0;
  }

  await res.send(
    {
    data: success
    }
  );

});

//*CLEAR DB
app.post('/clearDataInDB', async function(req, res, next) {

  var success = 0;
  var connBool = req.body.connBool;

  console.log("clearDataInDB: ");

  if(connBool == 1) {

  }
  else {

  }

  try {
    
    //*CLEAR DB
    await con.execute("SET FOREIGN_KEY_CHECKS = 0");
    await con.execute("TRUNCATE TABLE FILE");
    await con.execute("TRUNCATE TABLE ROUTE");
    await con.execute("TRUNCATE TABLE POINT");
    await con.execute("SET FOREIGN_KEY_CHECKS = 1");

    success = 1;

  }
  catch(e) {
    console.log("Query error: " + e);
  } 

  res.send(
	  {
		data: success
	  }
	);

});

//*DISPLAY DB STATUS
app.post('/displayInDBStat', async function(req, res, next) {

  var statArr = [];
  var connBool = req.body.connBool;

  console.log("clearDataInDB: ");

  if(connBool == 1) {
    try {
      
      //*CLEAR DB
      //let row = await con.execute("SELECT COUNT(*) as fileCount FROM FILE");
      let [row, field] = await con.execute("SELECT COUNT(*) as fileCount FROM FILE");
      let [row1, field2] = await con.execute("SELECT COUNT(*) as routeCount FROM ROUTE");
      let [row2, field3] = await con.execute("SELECT COUNT(*) as pointCount FROM POINT");

      statArr[0] = row[0].fileCount;
      statArr[1] = row1[0].routeCount;
      statArr[2] = row2[0].pointCount;

      console.log(row[0].fileCount + " " + row1[0].routeCount + " " + row2[0].pointCount);
    }
    catch(e) {
      console.log("Query error: " + e);
    } 
  }
  else {
    statArr = [];
  }

  res.send(
	  {
		data: statArr
	  }
	);

});


//*A3 FUNCTIONS
//*UPDATE FILE LOG WITH GPX JSONS
app.get('/updateFileLog', function(req, res) {
  const fileDir = './uploads/';
  var fileLogJSON = [];
  var gpxJSONStr;

  fs.readdir(fileDir, (err, files) => {

    if(err)
      console.log(err);
    else {
      files.forEach(file => {

        let tempFile = file;
        file = fileDir + file;  //add directory to file name

        if(file.includes(".gpx")) {  //only update .gpx files
          let gpxJSONStr = GPXParser.GPXFileLogJSON(file);
          if(gpxJSONStr != null) {
            let gpxJSON = JSON.parse(gpxJSONStr);
            gpxJSON["name"] = tempFile;

            fileLogJSON.push(gpxJSON);
            //console.log("gpxJSON: ", gpxJSON, "fileLogJSON: ", fileLogJSON, "\n");
          }
  
        }
      })
    }
    console.log("Updated File Log (", fileLogJSON.length, "): \n", fileLogJSON);
    res.send(
      {
        files: fileLogJSON
      }
    );  //return the file log JSON
  });
});

//*UPDATE GPX VIEW PANEL WITH TRACK AND ROUTES LIST JSONS
app.get('/updateGPXViewPanel/:filename', function(req, res) {
  const fileDir = './uploads/';
  const fileName = req.params.filename;
  console.log("updateGPXViewPanel:\n");
  console.log(fileName + "\n");

  var fileStr = fileDir + fileName;
  console.log(fileStr);
  var viewPanelJSON = [];

  let viewPanelStr = GPXParser.GPXViewPanelJSON(fileStr);
  viewPanelJSON = JSON.parse(viewPanelStr);

  console.log("View Panel JSON:");
  console.log(viewPanelJSON);

  console.log("Updated GPX View Panels: \n", viewPanelJSON);
  res.send(
    {
		data: viewPanelJSON
    }
  );
  //return the view panel JSON
  // fs.readFile(fileStr, function(err, vpInfo) {
    
  // });

});

//*UPDATE OTHER DATA SECTION
app.get('/updateOtherData/:filename', function(req, res) {
	const fileDir = './uploads/';
	var fileName = fileDir + req.params.filename;
	var dataName = req.query.otherName;
	var dI = req.query.dIndex;

	console.log("\nupdateOtherData:");
	console.log("fileName:" + fileName + "  otherName: " + req.query.otherName + "  dI: " + dI);

	var otherDataStr = "";

	otherDataStr = GPXParser.getOtherDataRadio(fileName, dataName, dI);

	console.log("Updated Other Data for " + req.query.otherName + dI + ":");
	console.log("Other Data: \n", otherDataStr);
	res.send(
	  {
		data: otherDataStr
	  }
	);
});

//*RENAME ROUTE/TRACK
app.post('/renameRouteTrack/:filename', async function(req, res) {

  console.log(req.body);
	const fileDir = './uploads/';
	var fileName = fileDir + req.params.filename;
	var dataName = req.body.dName;
	var dI = req.body.dIndex;
  var toName = req.body.nameVal;
  var ret_val = 0;
  var rteName;

  

	console.log("renameRouteTrack:");
	console.log("fileName:" + fileName + "  otherName: " + req.body.otherName + "  dI: " + dI + "  toName: " + toName);


  if(toName.length > 1) {
    rteName = GPXParser.getRouteAtIndex(fileName, dI);
    ret_val = GPXParser.renameRouteTrack(fileName, dataName, dI, toName);
  }

  console.log("ROUTE NAME: " + rteName);

  if(dataName.includes("Route")) {
    try {
      console.log("UPDATING ROUTE NAME IN DB: " + rteName + "  " + toName);
      await con.execute("UPDATE ROUTE SET route_name = '" + toName + "' WHERE route_name = '" + rteName + "'");
    }
    catch(e) {
      console.log("Query error: " + e);
    }
  }

	console.log("Updated Name for " + dataName + " " + dI + ":");
	res.send(
	  {
		data: ret_val
	  }
	);
});

//*ADD ROUTE TO GPX FILE
app.post('/addRouteToGPX/', async function(req, res) {

  console.log(req.body);
	const fileDir = './uploads/';

	var fileName = fileDir + req.body.fileName;
	var routeName = req.body.routeName;
  var ret_val = 0;

	console.log("addRouteToGPX:");
	console.log("fileName:" + fileName + "  routeName: " + routeName);

  if(fileName.length > 1 && routeName.length > 1) {
    ret_val = GPXParser.addRouteToFile(fileName, routeName);
  }
  if(ret_val == 1) {
    try {
      let testSTr = 'SELECT gpx_id FROM FILE WHERE file_name="' + req.body.fileName +'"';
      console.log(testSTr);
      let [row, field] = await con.execute('SELECT gpx_id FROM FILE WHERE file_name="' + req.body.fileName +'"');
      await con.execute("DELETE FROM ROUTE WHERE route_name='" + routeName + "';"); //overwrite routes with same name
      await con.execute("INSERT INTO ROUTE (route_name, route_len, gpx_id) VALUES ('" + routeName + "', '" + 0 + "', '" + row[0].gpx_id + "')");
    } catch(e) {
      console.log("Query error: " + e);
    }
    console.log("Added route to file: " + fileName);
  }
	res.send(
	  {
		data: ret_val
	  }
	);
});

//*ADD ROUTE TO GPX FILE
app.post('/addWaypToRT/', function(req, res) {

  console.log(req.body);
	const fileDir = './uploads/';

	var fName = fileDir + req.body.fName;
  var dName = req.body.dName;
  var dIndex = req.body.dIndex;
  var wName = req.body.wName;
  var wLat = req.body.wLat;
  var wLon = req.body.wLon;
  
  var ret_val = 0;

	console.log("addWaypToRT:");
	console.log("fName:" + fName + "  dName: " + dName + dIndex);

  if(fName.length > 1 && wName.length > 1 && dName.includes("Route")) {
    ret_val = GPXParser.addWaypointToFile(fName, dName, dIndex, wName, wLat, wLon);
  }
  if(ret_val == 1) {
    console.log("Added waypoint to file: " + fName);
  }
	res.send(
	  {
		data: ret_val
	  }
	);
});

//*CREATE NEW GPX FILLE
app.post('/createGPX/', function(req, res) {

  console.log(req.body);
	const fileDir = './uploads/';

	var fileName = fileDir + req.body.fileName;
	var creatorName = req.body.creatorName;
  var ret_val = 0;

  

	console.log("createGPX:");
	console.log("fileName:" + fileName + "  creatorName: " + creatorName);

  if(fileName.length > 1 && creatorName.length > 1 && fileName.includes(".gpx")) {
    ret_val = GPXParser.createGPXFile(fileName, creatorName);
  }
  if(ret_val == 1) {
    console.log("Created GPX File: " + fileName);
  }
	res.send(
	  {
		data: ret_val
	  }
	);
});

//*GET ROUTE/TRACKS BETWEEN 2 POINTS
app.get('/getPaths/', function(req, res) {
	const fileDir = './uploads/';

  var p_1lat = req.query.point1_lat;
  var p_1lon = req.query.point1_lon;
  var p_2lat = req.query.point2_lat;
  var p_2lon = req.query.point1_lon;
  var d_1 = req.query.d;

  var pathJSONArr = [];
  var pJSON = "";
	
  fs.readdir(fileDir, (err, files) => {
    if(err)
      console.log(err);
    else {
      files.forEach(file => {

        let temp = file;
        file = fileDir + file;  //add directory to file name

        if(file.includes(".gpx")) {  //only update .gpx files
          let currPathJSON = GPXParser.getPathJSON(file, p_1lat, p_1lon, p_2lat, p_2lon, d_1);
          if(currPathJSON != null) {
            console.log(currPathJSON);
            pathJSONArr.push(JSON.parse(currPathJSON));
           
            //console.log("gpxJSON: ", gpxJSON, "fileLogJSON: ", fileLogJSON, "\n");
          }
        }
      })
    }
    console.log("getPaths JSON:");
    console.log(pathJSONArr);
    console.log("Retrieved paths (getPaths):");
    res.send(
      {
        data: pathJSONArr
      }
    );  //return the file log JSON
  });
});


app.listen(portNum);
console.log('Running app at localhost: ' + portNum);