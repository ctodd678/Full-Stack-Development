#ifndef GPX_PARSER_H
#define GPX_PARSER_H

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlschemastypes.h>
#include "LinkedListAPI.h"

//PI number for haversine calculation
#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

//GPXData struct
typedef struct  {
	char 	name[256];	//GPXData name
	char	value[]; //GPXData value
} GPXData;

typedef struct {
    char* name;
    double longitude;
    double latitude;
    List* otherData;
} Waypoint;

typedef struct {
    char* name;
    List* waypoints;
    List* otherData;
} Route;

typedef struct {
    List* waypoints;
} TrackSegment;

typedef struct {
    char* name;
    List* segments;
    List* otherData;
} Track;


typedef struct {
    
    char namespace[256];
    double version;
    char* creator;

    List* waypoints;
    List* routes;
    List* tracks;
} GPXdoc;




GPXdoc* createGPXdoc(char* fileName);

char* GPXdocToString(GPXdoc* doc);

void deleteGPXdoc(GPXdoc* doc);

//Total number of waypoints in the GPX file
int getNumWaypoints(const GPXdoc* doc);

//Total number of routes in the GPX file
int getNumRoutes(const GPXdoc* doc);

//Total number of tracks in the GPX file
int getNumTracks(const GPXdoc* doc);

//Total number of segments in all tracks in the document
int getNumSegments(const GPXdoc* doc);

//Total number of GPXData elements in the document
int getNumGPXData(const GPXdoc* doc);

// Function that returns a waypoint with the given name.  If more than one exists, return the first one.  
// Return NULL if the waypoint does not exist
Waypoint* getWaypoint(const GPXdoc* doc, char* name);
// Function that returns a track with the given name.  If more than one exists, return the first one. 
// Return NULL if the track does not exist 
Track* getTrack(const GPXdoc* doc, char* name);
// Function that returns a route with the given name.  If more than one exists, return the first one.  
// Return NULL if the route does not exist
Route* getRoute(const GPXdoc* doc, char* name);



/* ******************************* A2 functions *************************** */


//Module 1

GPXdoc* createValidGPXdoc(char* fileName, char* gpxSchemaFile);

bool validateGPXDoc(GPXdoc* doc, char* gpxSchemaFile);

bool writeGPXdoc(GPXdoc* doc, char* fileName);


//Module 2

float getRouteLen(const Route *rt);
float getTrackLen(const Track *tr);
float round10(float len);
int numRoutesWithLength(const GPXdoc* doc, float len, float delta);
int numTracksWithLength(const GPXdoc* doc, float len, float delta);
bool isLoopRoute(const Route* route, float delta);
bool isLoopTrack(const Track *tr, float delta);
List* getRoutesBetween(const GPXdoc* doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta);
List* getTracksBetween(const GPXdoc* doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta);


//Module 3

char* trackToJSON(const Track *tr);
char* routeToJSON(const Route *rt);
char* routeListToJSON(const List *list);
char* trackListToJSON(const List *list);
char* GPXtoJSON(const GPXdoc* gpx);

void addWaypoint(Route *rt, Waypoint *pt);
void addRoute(GPXdoc* doc, Route* rt);

GPXdoc* JSONtoGPX(const char* gpxString);
Waypoint* JSONtoWaypoint(const char* gpxString);
Route* JSONtoRoute(const char* gpxString);


/* ******************************* List helper functions *************************** */

void deleteGpxData( void* data);
char* gpxDataToString( void* data);
int compareGpxData(const void *first, const void *second);

void deleteWaypoint(void* data);
char* waypointToString( void* data);
int compareWaypoints(const void *first, const void *second);

void deleteRoute(void* data);
char* routeToString(void* data);
int compareRoutes(const void *first, const void *second);

void deleteTrackSegment(void* data);
char* trackSegmentToString(void* data);
int compareTrackSegments(const void *first, const void *second);

void deleteTrack(void* data);
char* trackToString(void* data);
int compareTracks(const void *first, const void *second);

//*A2 FUNCTIONS
xmlDoc     *gpxDocTree              (GPXdoc *);
void        addOtherDataToXML       (xmlNode *, List *, xmlNsPtr);
void        addWaypointsToXML       (xmlNode *, List *, xmlNsPtr, char[50]);
bool        validateXmlTree         (xmlDoc *, char*);
bool        validateWaypoints       (List *);
bool        validateOtherData       (List *);
float       calcHaversine           (double, double, double, double);
void        fakeDelete              (void* );
int         getTrackWaypoints       (Track *);

//*A3 FUNCTIONS
char       *GPXFileLogJSON          (char *);
char       *GPXViewPanelJSON        (char *);
char       *getOtherDataRadio       (char *, char *, int );
int         renameRouteTrack        (char *, char *, int, char*);
char       *getPathJSON             (char *, float, float, float, float, float);
int         createGPXFile           (char *, char *);
int         addRouteToFile          (char *, char *);
int         addWaypointToFile       (char *, char *, int, char*, double, double);

//*A4 FUNCTIONS
char       *rteWaypToJSON           (char *, char *);
char       *waypToJSON              (const Waypoint*);
char       *getRouteAtIndex         (char *, int );



#endif