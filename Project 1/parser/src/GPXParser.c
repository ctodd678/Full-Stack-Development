#include "GPXParser.h"

GPXdoc* createGPXdoc(char* fileName) {

      FILE              *fp;
      GPXdoc            *newGPX;
      xmlNode           *xmlCurr = NULL, *xmlRoot = NULL, *xmlVal = NULL, *currChild = NULL;  
      xmlAttr           *xmlAttr = NULL;  
      xmlDoc            *xmlDoc = NULL;
      List              *wayp = NULL, *routes = NULL, *tracks = NULL;

      newGPX = malloc(sizeof(GPXdoc));    //initialize gpx
      if(newGPX == NULL) {
            printf("\nERORR! Invalid malloc in createGPXdoc (newGPX).");
            free(newGPX);
            return NULL;
      }

      if((fp = fopen(fileName, "r")) == NULL) {
            printf("\nERROR! Invalid file name or directory entered (createGPXdoc).");
            return NULL;
      }
      else {
            //*START PARSING
            if(!(xmlDoc = xmlReadFile(fileName, NULL, 0))) {
                  printf("\nERROR! Could not parse XML file %s (createGPXdoc)." , fileName);
                  return NULL;
            }

            xmlRoot = xmlDocGetRootElement(xmlDoc);

            if(xmlRoot->ns->href != NULL)
                  strcpy(newGPX->namespace, (char*)xmlRoot->ns->href);  //*SET NAMESPACE

            //*GET VERSION & CREATOR
            xmlCurr = xmlRoot;
            while(xmlCurr != NULL) {
            
                  xmlAttr = xmlRoot->properties;
                  while(xmlAttr != NULL) {
                        xmlVal = xmlAttr->children;

                        if(strcmp((char*)xmlAttr->name, "version") == 0) {
                              newGPX->version = strtod((char*)xmlVal->content, NULL);    //SET VERSION
                        }
                        else if(strcmp((char*)xmlAttr->name, "creator") == 0) {
                              newGPX->creator = malloc(strlen((char*)xmlVal->content) + 1);
                              strcpy(newGPX->creator, (char*)xmlVal->content);           //SET CREATOR
                        }
                        xmlAttr = xmlAttr->next;    //set next xml attribute
                  }
                  xmlCurr = xmlCurr->next;      //set next xml node
            }

            //*INITIALIZE WAYPOINTS, ROUTES & TRACKS
            wayp = initializeList(waypointToString, deleteWaypoint, compareWaypoints);
		routes = initializeList(routeToString, deleteRoute, compareRoutes);
		tracks = initializeList(trackToString, deleteTrack, compareTracks);
            

            xmlCurr = xmlRoot->children;
            while(xmlCurr != NULL) {
                  if(strcmp((char*)xmlCurr->name, "wpt") == 0) {  //*GET WAYPOINT
                        //INITIALIZE DATA
				List *otherD = initializeList(gpxDataToString, deleteGpxData, compareGpxData);
				Waypoint *temp = malloc(sizeof(Waypoint));
                        temp->name = malloc(sizeof(char) * 12);
                        strcpy(temp->name, " ");

                        xmlAttr = xmlCurr->properties;
                        while(xmlAttr != NULL) {      //*GET LAT & LON

                              xmlVal = xmlAttr->children;
                              if(strcmp((char*)xmlAttr->name, "lat"))
                                    temp->longitude = strtod((char*)xmlVal->content, NULL);	//set lon in waypoint
                              else if(strcmp((char*)xmlAttr->name, "lon"))
                                    temp->latitude = strtod((char*)xmlVal->content, NULL);	//set lat in Waypoint

                              xmlAttr = xmlAttr->next;    //set next xml attribute
                        }

                        currChild = xmlCurr->children;
                        while(currChild != NULL) {	//*GET NAME OF WAYPOINT

                              if(strcmp((char*)currChild->name, "name") == 0) {
                                    free(temp->name);
						temp->name = malloc(strlen((char*)currChild->children->content) + 1);
						strcpy(temp->name, (char*)currChild->children->content);	//set name in Waypoint
                              }
                              else if(currChild->type == XML_ELEMENT_NODE) {
                                    GPXData *otherW = malloc(sizeof(*otherW) + sizeof(char[strlen((char*)currChild->children->content) + 1]));

						strcpy(otherW->name, (char*)currChild->name);
						strcpy(otherW->value, (char*)currChild->children->content);
						insertBack(otherD, otherW);
                              }

                              currChild = currChild->next;
                        }

				temp->otherData = otherD;	//set otherData in Waypoint
                       	insertBack(wayp, temp);		//*INSERT WAYPOINT IN GPXDOC
                  }
                  else if(strcmp((char*)xmlCurr->name, "rte") == 0) {	//*GET ROUTE

                        //INITIALIZE DATA
				List *routeOther = initializeList(gpxDataToString, deleteGpxData, compareGpxData);
				List *routeWayp = initializeList(waypointToString, deleteWaypoint, compareWaypoints);
				Route	*currRoute = malloc(sizeof(Route));
                        currRoute->name = malloc(sizeof(char) * 12);
                        strcpy(currRoute->name, " ");
                        
                        currChild = xmlCurr->children;
                        while(currChild != NULL) {	//*GET NAME OF ROUTE

                              if(strcmp((char*)currChild->name, "name") == 0) {
                                    free(currRoute->name);
						currRoute->name = malloc(strlen((char*)currChild->children->content) + 1);
						strcpy(currRoute->name, (char*)currChild->children->content);	//*SET ROUTE NAME
                              }
					else if(strcmp((char*)currChild->name, "rtept") == 0) {

                                    //INITIALIZE DATA
                                    List *waypOther = initializeList(gpxDataToString, deleteGpxData, compareGpxData);
						Waypoint *rWayp = malloc(sizeof(Waypoint));
                                    rWayp->name = malloc(sizeof(char) * 12);
                                    strcpy(rWayp->name, " ");

						xmlAttr = currChild->properties;
                       			while(xmlAttr != NULL) {      //*GET LAT & LON

                             			xmlVal = xmlAttr->children;
							if(strcmp((char*)xmlAttr->name, "lat"))
                                    		rWayp->longitude = strtod((char*)xmlVal->content, NULL);	//set lon in waypoint
                              		else if(strcmp((char*)xmlAttr->name, "lon"))
                                    		rWayp->latitude = strtod((char*)xmlVal->content, NULL);	//set lat in Waypoint
                              
                              		xmlAttr = xmlAttr->next;    //set next xml attribute
                        		}

						
						xmlNode *childChild = currChild->children;
						while(childChild != NULL) {

							if(strcmp((char*)childChild->name, "name") == 0) {
                                                free(rWayp->name);
								rWayp->name = malloc(strlen((char*)childChild->children->content) + 1);
								strcpy(rWayp->name, (char*)childChild->children->content);
							}
                                          else if(childChild->type == XML_ELEMENT_NODE) {
                                                GPXData *currOtherW = malloc(sizeof(*currOtherW) + sizeof(char[strlen((char*)childChild->children->content) + 1]));
						            strcpy(currOtherW->name, (char*)childChild->name);
						            strcpy(currOtherW->value, (char*)childChild->children->content);
						            insertBack(waypOther, currOtherW);
                                          }
							childChild = childChild->next;
						}

						rWayp->otherData = waypOther;
						insertBack(routeWayp, rWayp);	//*INSERT WAYPOINT IN ROUTE WAYPOINTS	
                              }
					else if(currChild->type == XML_ELEMENT_NODE) {
						GPXData *currOther = malloc(sizeof(*currOther) + sizeof(char[strlen((char*)currChild->children->content) + 1]));
						strcpy(currOther->name, (char*)currChild->name);
						strcpy(currOther->value, (char*)currChild->children->content);
						insertBack(routeOther, currOther);
                              }
					

                              currChild = currChild->next;
                        }


				currRoute->otherData = routeOther;		//*SET ROUTE OTHER DATA
				currRoute->waypoints = routeWayp;		//*SET ROUTE WAYPOINTS

                       	insertBack(routes, currRoute);		//*INSERT ROUTE IN GPXDOC
                  }
                  else if(strcmp((char*)xmlCurr->name, "trk") == 0) {	//*GET TRACK
				//printf("Name: %s\n" , (char*)xmlCurr->name);
				List *trackOther = initializeList(gpxDataToString, deleteGpxData, compareGpxData);
				List *trackSeg = initializeList(trackSegmentToString, deleteTrackSegment, compareTrackSegments);
				Track	*currTrack = malloc(sizeof(Track));
                        currTrack->name = malloc(sizeof(char) * 12);
                        strcpy(currTrack->name, " ");
                        
                        currChild = xmlCurr->children;
                        while(currChild != NULL) {	
                              
                              if(strcmp((char*)currChild->name, "name") == 0) { 	//*GET NAME OF TRACK
                                    free(currTrack->name);
						currTrack->name = malloc(strlen((char*)currChild->children->content) + 1);
						strcpy(currTrack->name, (char*)currChild->children->content);	//*SET TRACK NAME
                              }
					else if(strcmp((char*)currChild->name, "trkseg") == 0) {			//*GET TRACK SEGMENT
                                    List *segList = initializeList(waypointToString, deleteWaypoint, compareWaypoints);
						TrackSegment *currSeg = malloc(sizeof(TrackSegment));
						
						
						xmlNode *childChild = currChild->children;
						while(childChild != NULL) {

							if(strcmp((char*)childChild->name, "trkpt") == 0) {		//*GET TRACK POINT
                                                List *trackWaypOther = initializeList(gpxDataToString, deleteGpxData, compareGpxData);
								Waypoint *tWayp = malloc(sizeof(Waypoint));
                                                tWayp->name = malloc(sizeof(char) * 12);
                                                strcpy(tWayp->name, " ");

								xmlAttr = childChild->properties;
                       					while(xmlAttr != NULL) {      		//*GET LAT & LON

                             					xmlVal = xmlAttr->children;

									if(strcmp((char*)xmlAttr->name, "lat"))
                                    				tWayp->longitude = strtod((char*)xmlVal->content, NULL);	//set lon in trackpoint
                              				else if(strcmp((char*)xmlAttr->name, "lon"))
                                    				tWayp->latitude = strtod((char*)xmlVal->content, NULL);	//set lat in trackpoint
                              
                              				xmlAttr = xmlAttr->next;    //set next xml attribute
                        				}

								xmlNode *tpChild = childChild->children;
								while(tpChild != NULL) {
									if(strcmp((char*)tpChild->name, "name") == 0){		//*GET TRACK POINT NAME
                                                            free(tWayp->name);
										tWayp->name = malloc(strlen((char*)tpChild->children->content) + 1);
										strcpy(tWayp->name, (char*)tpChild->children->content);
									}
                                                      else if(tpChild->type == XML_ELEMENT_NODE) {
						                        GPXData *currTrWaOther = malloc(sizeof(*currTrWaOther) + sizeof(char[strlen((char*)tpChild->children->content) + 1]));
						                        strcpy(currTrWaOther->name, (char*)tpChild->name);
						                        strcpy(currTrWaOther->value, (char*)tpChild->children->content);
						                        insertBack(trackWaypOther, currTrWaOther);
                                                      }
									tpChild = tpChild->next;
								}
								
								tWayp->otherData = trackWaypOther;
								insertBack(segList, (Waypoint*)tWayp);
							}
							childChild = childChild->next;
						}
                                    currSeg->waypoints = segList;
						insertBack(trackSeg, currSeg);	//*INSERT WAYPOINT IN ROUTE WAYPOINTS
                              }
					else if(currChild->type == XML_ELEMENT_NODE) {
						GPXData *currTrackOther = malloc(sizeof(*currTrackOther) + sizeof(char[strlen((char*)currChild->children->content) + 1]));

						strcpy(currTrackOther->name, (char*)currChild->name);
						strcpy(currTrackOther->value, (char*)currChild->children->content);
						insertBack(trackOther, currTrackOther);
                              }

                              currChild = currChild->next;
                        }

				currTrack->otherData = trackOther;		//*SET TRACK OTHER DATA
				currTrack->segments = trackSeg;		//*SET TRACK SEGMENTS

                       	insertBack(tracks, currTrack);		//*INSERT TRACK IN GPXDOC
                  }
                  
                  
                  
                  //printf("\n---------\n");
                  xmlCurr = xmlCurr->next;
            }
            newGPX->waypoints = wayp;	//*SET WAYPOINTS IN GPX
		newGPX->routes = routes;	//*SET ROUTES IN GPX
		newGPX->tracks = tracks;	//*SET TRACKS IN GPX

            fclose(fp);
            xmlFreeDoc(xmlDoc);
            xmlCleanupParser();
      }
      return newGPX;
}

char* GPXdocToString(GPXdoc* doc) {

      GPXdoc *g = doc;
      char *w = toString(g->waypoints);
      char *r = toString(g->routes);
      char *t = toString(g->tracks);
      char *str = malloc(sizeof(char) * 8012);

      sprintf(str, "%s\n%f\n%s\n\n%s\n\n%s\n\n%s" , g->namespace, g->version, g->creator, w, r, t);

      free(w);
      free(r);
      free(t);

      return str;
}

void deleteGPXdoc(GPXdoc* doc) {

      GPXdoc *temp = doc;

      free(temp->creator);

      freeList(temp->waypoints);
      freeList(temp->routes);
      freeList(temp->tracks);
      free(temp);
}

//*GET FUNCTIONS
//Total number of waypoints in the GPX file
int getNumWaypoints(const GPXdoc* doc) {

      if(doc == NULL || getLength(doc->waypoints) == 0)
            return 0;

      List *wList = doc->waypoints;
      ListIterator listI = createIterator(wList);
      Waypoint *currW = (Waypoint*)wList->head;
      int count = 0;

      while(currW != NULL) {
            count++;
            currW = nextElement(&listI);
      }
      count--;
      return count;
}

//Total number of routes in the GPX file
int getNumRoutes(const GPXdoc* doc) {

      if(doc == NULL || getLength(doc->routes) == 0)
            return 0;

      List *rList = doc->routes;
      ListIterator listI = createIterator(rList);
      Route *currR = (Route*)rList->head;
      int count = 0;

      while(currR != NULL) {
            count++;
            currR = nextElement(&listI);
      }
      count--;
      return count;
}

//Total number of tracks in the GPX file
int getNumTracks(const GPXdoc* doc) {

      if(doc == NULL || getLength(doc->tracks) == 0)
            return 0;
      
      List *tList = doc->tracks;
      ListIterator listI = createIterator(tList);
      Track *currT = (Track*)tList->head;
      int count = 0;

      while(currT != NULL) {
            count++;
            currT = nextElement(&listI);
      }
      count--;
      return count;
}

//Total number of segments in all tracks in the document
int getNumSegments(const GPXdoc* doc) {

      if(doc == NULL || getLength(doc->tracks) == 0)
            return 0;
      
      List *tList = doc->tracks;
      ListIterator listI = createIterator(tList);
      Track *currT = (Track*)tList->head->data;
      currT = nextElement(&listI);
      int count = 0;

      while(currT != NULL) {
            count = count + getLength(currT->segments);
            currT = nextElement(&listI);
      }
      return count;
}

//Total number of GPXData elements in the document
int getNumGPXData(const GPXdoc* doc) {

      if(doc == NULL)
            return 0;

      int count = 0;
      List *wList = doc->waypoints;
      List *rList = doc->routes;


      if(getLength(doc->waypoints) == 0) //*SKIP WAYPOINTS
            goto routes;

      ListIterator listI = createIterator(wList);
      Waypoint *currW = (Waypoint*)wList->head->data;
      currW = nextElement(&listI);

      
      //TRAVERSE WAYPOINTS
      while(currW != NULL) {
            count = count + getLength(currW->otherData);    //count otherdata list

            if(strcmp(currW->name, " ") != 0)
                  count++;                                  //count name

            currW = nextElement(&listI);                    //set next waypoint
      }
      routes:

      
      if(getLength(doc->routes) == 0)     //*SKIP ROUTES
            goto tracks;

      listI = createIterator(rList);
      Route *currR = (Route*)rList->head->data;
      currR = nextElement(&listI);

      wList = currR->waypoints;
      ListIterator listI_W = createIterator(wList);
      currW = (Waypoint*)wList->head->data;
      currW = nextElement(&listI_W);

      //TRAVERSE ROUTES
      while(currR != NULL) {

            count = count + getLength(currR->otherData);          //count otherData list
            if(strcmp(currR->name, " ") != 0)
                  count++;                                        //count route name

            while(currW != NULL) {
                  count = count + getLength(currW->otherData);    //count otherdata list

                  if(strcmp(currW->name, " ") != 0)
                        count++;                                  //count waypoint name

                  currW = nextElement(&listI_W);                  //set next waypoint
            }
            currR = nextElement(&listI);                          //set next route
      }
      tracks:


      if(getLength(doc->tracks) == 0)     //*SKIP TRACKS
            goto end;

      Node *currNodeT;
      Node *currNodeS;
      Node *currNodeW;

      currNodeT = doc->tracks->head;
      while(currNodeT != NULL) {
            Track *currTrk = currNodeT->data;

            count = count + currTrk->otherData->length;    //count otherData list
            if(strcmp(currTrk->name, " ") != 0)
                  count++;

            currNodeS = currTrk->segments->head;
            while(currNodeS != NULL) {
                  TrackSegment *currSeg = currNodeS->data;

                  currNodeW = currSeg->waypoints->head;
                  while(currNodeW != NULL) {
                        Waypoint *currWayp = currNodeW->data;

                        count = count + currWayp->otherData->length;
                        
                        if(strcmp(currWayp->name, " ") != 0)
                              count++;

                        currNodeW = currNodeW->next;
                  }
                  currNodeS = currNodeS->next;
            }
            currNodeT = currNodeT->next;
      }

      end:

      return count;
}


// Function that returns a waypoint with the given name.  If more than one exists, return the first one.  
// Return NULL if the waypoint does not exist
Waypoint* getWaypoint(const GPXdoc* doc, char* name) {

      if(doc == NULL || getLength(doc->waypoints) == 0)
            return NULL;

      List *wList = doc->waypoints;
      ListIterator listI = createIterator(wList);
      Waypoint *currW = (Waypoint*)wList->head;
      currW = nextElement(&listI);

      while(currW != NULL) {
            if(strcmp((char*)currW->name, name) == 0) { 
                  return currW;
            }
            currW = nextElement(&listI);
      }
      return NULL;
}
// Function that returns a track with the given name.  If more than one exists, return the first one. 
// Return NULL if the track does not exist 
Track* getTrack(const GPXdoc* doc, char* name) {
      if(doc == NULL || getLength(doc->tracks) == 0)
            return 0;

      List *tList = doc->tracks;
      ListIterator listI = createIterator(tList);
      Track *currT = (Track*)tList->head;

      while(currT != NULL) {
            if(strcmp(currT->name, name) == 0) {
                  return currT;
            }
            currT = nextElement(&listI);
      }

      return NULL;
}
// Function that returns a route with the given name.  If more than one exists, return the first one.  
// Return NULL if the route does not exist
Route* getRoute(const GPXdoc* doc, char* name) {

      if(doc == NULL || getLength(doc->routes) == 0)
            return 0;

      List *rList = doc->routes;
      ListIterator listI = createIterator(rList);
      Route *currR = (Route*)rList->head;

      while(currR != NULL) {
            if(strcmp(currR->name, name) == 0) {
                  return currR;
            }
            currR = nextElement(&listI);
      }
      
      return NULL;
}


/* ******************************* List helper functions  - MUST be implemented *************************** */

void deleteGpxData( void* data) {

      GPXData *gpx = data;

      free(gpx);
}
char* gpxDataToString(void* data) {

      GPXData *gpx = data;

      char *temp = malloc(sizeof(char) * 256);
      char *nameStr = gpx->name;
      char *valueStr = gpx->value;
      sprintf(temp, "%s   %s" , nameStr, valueStr);

      return temp;
}
int compareGpxData(const void *first, const void *second) {

      GPXData *gpx1 = (GPXData*)first;
      GPXData *gpx2 = (GPXData*)second;

      return strcmp(gpx1->name, gpx2->name);
}

void deleteWaypoint(void* data) {
      Waypoint *w1 = data;

      free(w1->name);
      freeList(w1->otherData);
      free(w1);
}
char* waypointToString( void* data) {
      
      Waypoint *w1 = data;

      char *nameD = w1->name;
      double longD = w1->longitude;
      double lattD = w1->latitude;
      char *otherD = toString(w1->otherData);

      char *temp = malloc(strlen(nameD) + strlen(otherD) + 50);

      sprintf(temp, "%s\n   %f   %f%s\n",   nameD, lattD, longD, otherD);    //TODO: add list 

      free(otherD);

      return temp;
}
int compareWaypoints(const void *first, const void *second) {

      Waypoint *w1 = (Waypoint*)first;
      Waypoint *w2 = (Waypoint*)second;
      return strcmp(w1->name, w2->name);
}

void deleteRoute(void* data) {

      Route *r1 = data;

      free(r1->name);
      freeList(r1->waypoints);
      freeList(r1->otherData);
      free(r1);
}
char* routeToString(void* data) {

      Route *r1 = data;

      char *n1 = r1->name;
      char *w1 = toString(r1->waypoints);
      char *otherD = toString(r1->otherData);

      char *temp = malloc(strlen(n1) + strlen(w1) + strlen(otherD) + 2);
      sprintf(temp, "%s\n%s%s", n1, w1, otherD);

      free(w1);
      free(otherD);

      return temp;
}
int compareRoutes(const void *first, const void *second) {

      Route *r1 = (Route*)first;
      Route *r2 = (Route*)second;

      return strcmp(r1->name, r2->name);
}

void deleteTrackSegment(void* data) {

      TrackSegment *t1 = (TrackSegment*)data;

      //t1->waypoints = NULL;
      freeList(t1->waypoints);
      free(t1);
}
char* trackSegmentToString(void* data) {

      TrackSegment *t1 = data;
      
      return toString(t1->waypoints);
}
int compareTrackSegments(const void *first, const void *second) {

      TrackSegment *t1 = (TrackSegment*)first;
      TrackSegment *t2 = (TrackSegment*)second;

      return strcmp(toString(t1->waypoints), toString(t2->waypoints));
}

void deleteTrack(void* data) {

      Track *t1 = (Track*)data;

	free(t1->name);
      freeList(t1->segments);
	freeList(t1->otherData);
      free(t1);
}
char* trackToString(void* data) {

      Track *track1 = (Track*)data;
      

      char *tName = track1->name;
      char *tSeg = toString(track1->segments);
      char *tOther = toString(track1->otherData);

      char *temp = malloc(strlen(tName) + strlen(tSeg) + strlen(tOther) + 10);

      sprintf(temp, "%s   %s   %s", tName, tSeg, tOther);

      free(tSeg);
      free(tOther);

      return temp;
}
int compareTracks(const void *first, const void *second) {
      Track *t1 = (Track*)first;
      Track *t2 = (Track*)second;

      return strcmp(t1->name, t2->name);
}

/* ******************************* A2 functions  - MUST be implemented *************************** */
//Module 1

GPXdoc* createValidGPXdoc(char* fileName, char* gpxSchemaFile) {

      xmlDoc *xmlDoc = NULL;
      GPXdoc *validDoc = NULL;

      
      if(fileName == NULL || gpxSchemaFile == NULL) {
            printf("ERROR! Invalid fileName or gpxSchemaFile (createValidGPXdoc).\n");
            return NULL;
      }

      if(!(xmlDoc = xmlReadFile(fileName, NULL, 0))) {
            printf("\nERROR! Could not parse XML file %s (createValidGPXdoc)." , fileName);
            xmlFreeDoc(xmlDoc);
            return NULL;
      }

      if(validateXmlTree(xmlDoc, gpxSchemaFile))
            validDoc = createGPXdoc(fileName);
      else 
            return NULL;

      //*VALIDATE GPX DOC then, RETURN
      if(!(validateGPXDoc(validDoc, gpxSchemaFile)))
            return NULL;

      xmlFreeDoc(xmlDoc);
      xmlCleanupParser();

      return validDoc;
}

bool validateGPXDoc(GPXdoc* doc, char* gpxSchemaFile) {

      xmlDoc *xmlGpxTree = NULL;
      Node *currNode = NULL;

      if (doc == NULL || gpxSchemaFile == NULL)
            return false;

      xmlGpxTree = gpxDocTree(doc);

      if(!(validateXmlTree(xmlGpxTree, gpxSchemaFile)))
            return false;
      
      //*VALIDATE WAYPOINTS
      validateWaypoints(doc->waypoints);
      

      //*VALIDATE ROUTES
      Route *currRte;
      currNode = doc->routes->head;
      while(currNode != NULL) {

            currRte = (Route*)currNode->data;

            if(currRte->name == NULL)
                  return false;
      
            if(!(validateWaypoints(currRte->waypoints)))             //*VALIDATE ROUTE WAYPOINTS
                  return false;
      
            if(!(validateOtherData(currRte->otherData)))             //*VALIDATE ROUTE OTHER DATA
                  return false;

            currNode = currNode->next;
      }

      //*VALIDATE TRACKS
      Track *currTrk;
      currNode = doc->tracks->head;
      while(currNode != NULL) {

            currTrk = (Track*)currNode->data;

            if(currTrk->name == NULL)
                  return false;

            if(!(validateOtherData(currTrk->otherData)))             //*VALIDATE TRACK OTHER DATA
                  return false;

            //*SET TRACKSEGMENTS
            TrackSegment *currSeg;

            Node *currSegNode = currTrk->segments->head;
            while(currSegNode != NULL) {

                  currSeg = (TrackSegment*)currSegNode->data;

                  if(!(validateWaypoints(currSeg->waypoints)))             //*VALIDATE SEGMENT WAYPOINTS
                        return false;

                  currSegNode = currSegNode->next;
            }

            currNode = currNode->next;
      }

      xmlFreeDoc(xmlGpxTree);

      return true;
}

bool validateWaypoints(List *wayps) {

      Node *currNode;
      Waypoint *currWayp;

      currNode = wayps->head;
      while(currNode != NULL) {               //*TRAVERSE WAYPOINTS AND ADD TO XML TREE

            currWayp = (Waypoint*)currNode->data;

            if(currWayp->name == NULL)                     //CHECK NAME
                  return false;
            
            if(!(validateOtherData(currWayp->otherData)))  //*VALIDATE OTHER DATA
                  return false;

            currNode = currNode->next;
      }

      return true;
}

bool validateOtherData(List *otherD) {

      Node *currOther = otherD->head;
      while(currOther != NULL) {
            GPXData *currGPXData = currOther->data;

            if(currGPXData->name == NULL)
                  return false;
            if(currGPXData->value == NULL)
                  return false;
            
            currOther = currOther->next;
      }

      return true;
}

bool validateXmlTree(xmlDoc *tree, char* xsdFile) {

      xmlSchema *schema;
      xmlSchemaParserCtxtPtr ctxt;
      int ret_val;
      bool valid;

      if (tree == NULL || xsdFile == NULL)
            return false;


      xmlLineNumbersDefault(1);

      ctxt = xmlSchemaNewParserCtxt(xsdFile);
      xmlSchemaSetParserErrors(ctxt, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
      schema = xmlSchemaParse(ctxt);
      xmlSchemaFreeParserCtxt(ctxt);
      //xmlSchemaDump(stdout, schema); //To print schema dump

      xmlSchemaValidCtxtPtr ctxt2;
      ctxt2 = xmlSchemaNewValidCtxt(schema);
      xmlSchemaSetValidErrors(ctxt2, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
      ret_val = xmlSchemaValidateDoc(ctxt2, tree);    

      if(ret_val == 0)
            valid = true;
      else
            valid = false;
      
      xmlSchemaFree(schema);
      xmlSchemaFreeValidCtxt(ctxt2);

      return valid;
}

bool writeGPXdoc(GPXdoc* doc, char* fileName) {

      xmlDoc *xmlTree;
      int ret_val;
      bool write = false;

      if (doc == NULL || fileName == NULL)
            return false;

      xmlTree = gpxDocTree(doc);

      ret_val = xmlSaveFormatFileEnc(fileName, xmlTree, "UTF-8", 1);

      if(ret_val != -1)
            write = true;
      else
            write = false;

      xmlFreeDoc(xmlTree);
      xmlCleanupParser();

      return write;
}

xmlDoc *gpxDocTree(GPXdoc * doc) {

      xmlDoc  * xmlDocTree = NULL;
      xmlNode * xmlRoot = NULL;
      xmlNode * newXmlNode = NULL;
      xmlNode * newXmlNode1 = NULL;
      xmlNode * newXmlNode2 = NULL;
      xmlNode * newXmlNode3 = NULL;
      xmlNsPtr xmlNs = NULL;

      LIBXML_TEST_VERSION;

      //*CREATE ROOT NODE AND SET NAMESPACE
      xmlDocTree = xmlNewDoc(BAD_CAST "1.0");
      xmlRoot = xmlNewNode(xmlNs, BAD_CAST "gpx");
      xmlNs = xmlNewNs(xmlRoot, (xmlChar*)doc->namespace, NULL);	
      xmlSetNs(xmlRoot, xmlNs);
      xmlDocSetRootElement(xmlDocTree, xmlRoot);


      //*SET CREATOR AND VERSION
      char tempVer[100];
      sprintf(tempVer, "%.1f" , doc->version);
      xmlNewProp(xmlRoot, BAD_CAST "version", BAD_CAST tempVer);
      xmlNewProp(xmlRoot, BAD_CAST "creator", BAD_CAST doc->creator);


      //*SET WAYPOINTS, ROUTES & TRACKS
      addWaypointsToXML(xmlRoot, doc->waypoints, xmlNs, "wpt");    //*<--ADD WAYPOITNS
      
      Node *currNode;

      //*ADD ROUTES
      Route *currRte;
      currNode = doc->routes->head;
      while(currNode != NULL) {

            currRte = (Route*)currNode->data;
            
            newXmlNode = xmlNewNode(xmlNs, BAD_CAST "rte");
            xmlSetNs(newXmlNode, xmlNs);

            //*SET NAME
            newXmlNode1 = xmlNewNode(NULL, BAD_CAST "name");
            xmlSetNs(newXmlNode1, xmlNs);
            xmlAddChild(newXmlNode, newXmlNode1);

            newXmlNode2 = xmlNewText(BAD_CAST currRte->name);
            xmlSetNs(newXmlNode2, xmlNs);
            xmlAddChild(newXmlNode1, newXmlNode2);

            //*SET ROUTE WAYPOINTS AND OTHER DATA
            addOtherDataToXML(newXmlNode, currRte->otherData, xmlNs);                    //*SET OTHER DATA
            addWaypointsToXML(newXmlNode, currRte->waypoints, xmlNs, "rtept");    //*<--ADD WAYPOITNS
            
            xmlAddChild(xmlRoot, newXmlNode);   //*ADD ROUTE TO ROOT


            currNode = currNode->next;
      }

      //*ADD TRACKS
      Track *currTrk;
      currNode = doc->tracks->head;
      while(currNode != NULL) {

            currTrk = (Track*)currNode->data;
            
            newXmlNode = xmlNewNode(xmlNs, BAD_CAST "trk");
            xmlSetNs(newXmlNode, xmlNs);

            //*SET NAME
            newXmlNode1 = xmlNewNode(NULL, BAD_CAST "name");
            xmlSetNs(newXmlNode1, xmlNs);
            xmlAddChild(newXmlNode, newXmlNode1);

            newXmlNode2 = xmlNewText(BAD_CAST currTrk->name);
            xmlSetNs(newXmlNode2, xmlNs);
            xmlAddChild(newXmlNode1, newXmlNode2);

            //*SET OTHER DATA
            addOtherDataToXML(newXmlNode, currTrk->otherData, xmlNs);                    //*SET OTHER DATA

            //*SET TRACKSEGMENTS
            TrackSegment *currSeg;
            Node *currSegNode = currTrk->segments->head;
            while(currSegNode != NULL) {

                  currSeg = (TrackSegment*)currSegNode->data;

                  newXmlNode3 = xmlNewNode(xmlNs, BAD_CAST "trkseg");
                  xmlSetNs(newXmlNode3, xmlNs);
                  
                  //*SET SEGMENT WAYPOINTS
                  addWaypointsToXML(newXmlNode3, currSeg->waypoints, xmlNs, "trkpt");

                  xmlAddChild(newXmlNode, newXmlNode3);
                  currSegNode = currSegNode->next;
            }
            
            xmlAddChild(xmlRoot, newXmlNode);   //*ADD TRACK TO ROOT

            currNode = currNode->next;
      }

      return xmlDocTree;
}

void addWaypointsToXML(xmlNode * root, List *wayps, xmlNsPtr ns, char waypName[50]) {

      Node *currNode;
      Waypoint *currWayp;
      xmlNode * newXmlNode = NULL;
      xmlNode * newXmlNode1 = NULL;
      xmlNode * newXmlNode2 = NULL;

      if(wayps->length == 0) {
            return;
      }
      currNode = wayps->head;
      while(currNode != NULL) {               //*TRAVERSE WAYPOINTS AND ADD TO XML TREE

            currWayp = (Waypoint*)currNode->data;
            
            newXmlNode = xmlNewNode(ns, BAD_CAST waypName);
            xmlSetNs(newXmlNode, ns);

            //*SET LONGITUDE AND LATITUDE
            char tempLon[100], tempLat[100];
            sprintf(tempLon, "%.1f" , currWayp->longitude);
            sprintf(tempLat, "%.1f" , currWayp->latitude);
            xmlNewProp(newXmlNode, BAD_CAST "lat", BAD_CAST tempLat);
            xmlNewProp(newXmlNode, BAD_CAST "lon", BAD_CAST tempLon);

            //*SET NAME
            if(strcmp(currWayp->name, " ") != 0) {
                  newXmlNode1 = xmlNewNode(NULL, BAD_CAST "name");
                  xmlSetNs(newXmlNode1, ns);
                  xmlAddChild(newXmlNode, newXmlNode1);

                  newXmlNode2 = xmlNewText(BAD_CAST currWayp->name);
                  xmlSetNs(newXmlNode2, ns);
                  xmlAddChild(newXmlNode1, newXmlNode2);
            }
            
            addOtherDataToXML(newXmlNode, currWayp->otherData, ns);  //*SET OTHER DATA
            
            xmlAddChild(root, newXmlNode);   //*ADD WAYPOINT TO ROOT

            currNode = currNode->next;
      }
}

void addOtherDataToXML(xmlNode *toAdd, List *otherD, xmlNsPtr xmlNs) {
      
      xmlNode *newXml2 = NULL;
      xmlNode *newXml3 = NULL;

      if(otherD->length == 0) {
            return;
      }

      Node *currOther = otherD->head;
      while(currOther != NULL) {
            GPXData *currGPXData = currOther->data;

            newXml2 = xmlNewNode(NULL, BAD_CAST currGPXData->name);
            xmlSetNs(newXml2, xmlNs);
            newXml3 = xmlNewText(BAD_CAST currGPXData->value);
            xmlSetNs(newXml3, xmlNs);
            xmlAddChild(newXml2, newXml3);
            xmlAddChild(toAdd, newXml2);
            
            currOther = currOther->next;
      }
}

//* A2 MODULE 2

float getRouteLen(const Route *rt) {

      double length = 0.0;
      double lat_1, lon_1, lat_2, lon_2;

      if(rt == NULL) {
            return 0;
      }

      Node *currNode;

      currNode = rt->waypoints->head;

      if(currNode == NULL) {
            return 0;
      }

      Waypoint *currWayp = (Waypoint*)currNode->data;

      lat_1 = currWayp->latitude;
      lon_1 = currWayp->longitude;

      currNode = currNode->next;
      while(currNode != NULL) {
            Waypoint *currWayp = (Waypoint*)currNode->data;

            lat_2 = currWayp->latitude;
            lon_2 = currWayp->longitude;

            length = length + calcHaversine(lat_2, lon_2, lat_1, lon_1);

            lat_1 = currWayp->latitude;
            lon_1 = currWayp->longitude;
            
            currNode = currNode->next;
      }

      return (float)length;
}

float calcHaversine(double lat1, double lon1, double lat2, double lon2) {

      double d = 0.0;
      double R = 6371000;
      double a = 0.0, c = 0.0;
      double theta1, theta2;
      double deltaT, deltaL;

      theta1 = lat1 * (M_PI / 180);
      theta2 = lat2 * (M_PI / 180);
      deltaT = (lat2 - lat1) * (M_PI / 180);
      deltaL = (lon2 - lon1) * (M_PI / 180);
      
      a = sin(deltaT / 2) * sin(deltaT / 2) + cos(theta1) * cos(theta2) * sin(deltaL / 2) * sin(deltaL / 2);
      //a = pow(sin(deltaT / 2), 2) + cos(theta1) * cos(theta2) * pow(sin(deltaL), 2);

      c = 2 * atan2(sqrt(a), sqrt(1 - a));

      d = R * c;

      return (float)d;
}


float getTrackLen(const Track *tr) {

      double length = 0.0;
      double lat_1, lon_1, lat_2, lon_2;

      if(tr == NULL) {
            return 0;
      }

      Node *currNode;

      currNode = tr->segments->head;

      if(currNode == NULL) {
            return 0;
      }

      TrackSegment *currSegment = (TrackSegment*)currNode->data;

      while(currNode != NULL) {
            currSegment = (TrackSegment*)currNode->data;

            Node *currNode2;
            currNode2 = currSegment->waypoints->head;
            Waypoint *currWayp = (Waypoint*)currNode2->data;

            lat_1 = currWayp->latitude;
            lon_1 = currWayp->longitude;

            currNode2 = currNode2->next;
            while(currNode2 != NULL) {
                  Waypoint *currWayp = (Waypoint*)currNode2->data;

                  lat_2 = currWayp->latitude;
                  lon_2 = currWayp->longitude;

                  length = length + calcHaversine(lat_1, lon_1, lat_2, lon_2);

                  lat_1 = currWayp->latitude;
                  lon_1 = currWayp->longitude;
                  
                  currNode2 = currNode2->next;
            }

            currNode = currNode->next;
      }

      return (float)length;
}

float round10(float len) {

      float roun10 = 0.0;

      int x, y;

      x = ((int)len / 10) * 10;
      y = x + 10;

      if((int)len - x >= y - (int)len) {
            roun10 = y;
      }
      else {
            roun10 = x;
      }

      return roun10;
}

int numRoutesWithLength(const GPXdoc* doc, float len, float delta) {

      int count = 0;
      double length = 0.0;
      double min, max;

      min = len - delta;
      max = len + delta;

      if(doc == NULL || len < 0 || delta < 0)
            return 0;

      if(doc->routes->length == 0)
            return 0;
      
      Node *currNode;
      currNode = doc->routes->head;

      while(currNode != NULL) {

            Route *currRte = (Route*)currNode->data;
            
            length = getRouteLen(currRte);
            if(length <= max && length >= min)
                  count = count + 1;

            currNode = currNode->next;
      }

      return count;
}

int numTracksWithLength(const GPXdoc* doc, float len, float delta) {

      int count = 0;
      double length = 0.0;
      double min, max;

      min = len - delta;
      max = len + delta;

      if(doc == NULL || len < 0 || delta < 0)
            return 0;
      
      Node *currNode;
      currNode = doc->tracks->head;

      while(currNode != NULL) {

            Track *currTrk = (Track*)currNode->data;
            
            length = getTrackLen(currTrk);
            if(length <= max && length >= min)
                  count = count + 1;

            currNode = currNode->next;
      }

      return count;
}

bool isLoopRoute(const Route* route, float delta) {

      double dist;

      if(route == NULL || delta < 0)
            return false;
      
      if(route->waypoints == NULL)
            return false;
      
      if(route->waypoints->length < 4)
            return false;

      List *wayps = route->waypoints;
      Waypoint *w1 = (Waypoint*)wayps->head->data;
      Waypoint *w2 = (Waypoint*)wayps->tail->data;

      dist = calcHaversine(w1->latitude, w1->longitude, w2->latitude, w2->longitude);


      if(dist > delta)
            return false;

      return true;
}

bool isLoopTrack(const Track *tr, float delta) {

      int waypCount = 0;
      double dist = 0.0;


      if(tr == NULL || delta < 0)
            return false;


      if(tr->segments == NULL)
            return false;


      Node *currNode;
      List *wayps;


      currNode = tr->segments->head;
      TrackSegment *currSeg = (TrackSegment*)currNode->data;

      while(currNode != NULL) {     //*TRAVERSE SEGMENTS

            currSeg = (TrackSegment*)currNode->data;

            wayps = currSeg->waypoints;

            waypCount = waypCount + wayps->length;

            currNode = currNode->next;
      }

      if(waypCount < 4)
            return false;


      currSeg = (TrackSegment*)tr->segments->head->data;
      wayps = currSeg->waypoints;
      Waypoint *w1 = (Waypoint*)wayps->head->data;

      currSeg = (TrackSegment*)tr->segments->tail->data;
      wayps = currSeg->waypoints;
      Waypoint *w2 = (Waypoint*)wayps->tail->data;

      if(w1 != NULL && w2 != NULL)
            dist = calcHaversine(w1->latitude, w1->longitude, w2->latitude, w2->longitude);


      if(dist > delta)
            return false;

      return true;
}

void fakeDelete(void* data) {

      return;
}

List* getRoutesBetween(const GPXdoc* doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta) {

      double srcDist = 0;
      double destDist = 0;
      int count = 0;
      List *routes = NULL;

      if(doc == NULL || delta < 0)
            return NULL;

      if(doc->routes == NULL)
            return NULL;

      if(doc->routes->length == 0)
            return NULL;

      routes = initializeList(routeToString, fakeDelete, compareRoutes);

      Node *currNode;
      currNode = doc->routes->head;

      while(currNode != NULL) {

            Route *currRte = currNode->data;

            //*GET FIRST AND LAST POINTS OF ROUTE
            List *wayps = currRte->waypoints;

            if(wayps->length < 2) {
                  return NULL;
            }

            Waypoint *w1 = (Waypoint*)wayps->head->data;
            Waypoint *w2 = (Waypoint*)wayps->tail->data;

            srcDist = calcHaversine(sourceLat, sourceLong, w1->latitude, w1->longitude);
            destDist = calcHaversine(destLat, destLong, w2->latitude, w2->longitude);

            if(srcDist <= delta && destDist <= delta) {
                  insertBack(routes, currRte);  //*ADD TRACK IF WITHIN DELTA
                  count = count + 1;
            }

            currNode = currNode->next;
      }

      if(count == 0) {
            freeList(routes);
            return NULL;
      }

      return routes;
}

List* getTracksBetween(const GPXdoc* doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta) {

      double srcDist = 0;
      double destDist = 0;
      int count = 0;
      List *tracks = NULL;

      if(doc == NULL || delta < 0)
            return NULL;

      if(doc->tracks->length == 0)
            return NULL;


      tracks = initializeList(trackToString, deleteTrack, compareTracks);     //*INIT TRACK LIST

      Node *currNode;
      currNode = doc->tracks->head;

      while(currNode != NULL) {     //*TRAVERSE TRACKS

            Track *currTrk = currNode->data;

            List *segs = currTrk->segments;

            if(segs->length < 2) {
                  return NULL;
            }

            //*GET FIRST POINT OF FIRST SEGMENT
            TrackSegment *s1 = (TrackSegment*)segs->head->data;
            List *wayps1 = s1->waypoints;
            Waypoint *w1 = (Waypoint*)wayps1->head->data;

            //*GET LAST POINT OF LAST SEGMENT
            TrackSegment *s2 = (TrackSegment*)segs->tail->data;
            List *wayps2 = s2->waypoints;
            Waypoint *w2 = (Waypoint*)wayps2->tail->data;

            srcDist = calcHaversine(sourceLat, sourceLong, w1->latitude, w1->longitude);
            destDist = calcHaversine(sourceLong, destLong, w2->latitude, w2->longitude);

            if(srcDist <= delta && destDist <= delta) {
                  insertBack(tracks, currTrk);  //*ADD TRACK IF WITHIN DELTA
                  count = count + 1;
            }

            currNode = currNode->next;
      }

      if(count == 0) {
            freeList(tracks);
            return NULL;
      }


      return tracks;
}


//Module 3

int getTrackWaypoints(Track *t) {

      Node *currNode;
      int count = 0;

      currNode = t->segments->head;

      while(currNode != NULL) {

            TrackSegment *currSegment = (TrackSegment*)currNode->data;
            count = count + currSegment->waypoints->length;

            currNode = currNode->next;
      }

      return count;
}

char* trackToJSON(const Track *tr) {

      char* trkJSON;
      char* name;
      float trackLen;
      int numPoints = 0;
      bool loopState;

      if(tr == NULL) {
            trkJSON = malloc(sizeof(char) * 3);
            strcpy(trkJSON, "{}");
            return trkJSON;
      }
      
      if(strcmp(tr->name, " ") != 0)
            name = tr->name;
      else
            name = "None";

      trackLen = getTrackLen(tr);
      numPoints = getTrackWaypoints((Track*)tr);
      trackLen = round10(trackLen);
      
      loopState = isLoopTrack(tr, 10);

      trkJSON = malloc(sizeof(char*) * (strlen(name) + strlen(loopState ? "true" : "false") + 100));

      sprintf(trkJSON, "{\"name\":\"%s\",\"numPoints\":%d,\"len\":%.1f,\"loop\":%s}", name, numPoints, trackLen, loopState ? "true" : "false");
      return trkJSON;
}

char* routeToJSON(const Route *rt) {

      char* rteJSON;
      char* name;
      float routeLen;
      int numPoints;
      bool loopState;

      if(rt == NULL) {
            rteJSON = malloc(sizeof(char) * 3);
            strcpy(rteJSON, "{}");
            return rteJSON;
      }
      
      if(strcmp(rt->name, " ") != 0)
            name = rt->name;
      else
            name = "None";

      routeLen = getRouteLen(rt);
      routeLen = round10(routeLen);
      
      loopState = isLoopRoute(rt, 10);

      numPoints = rt->waypoints->length;

      rteJSON = malloc(sizeof(char*) * (strlen(name) + strlen(loopState ? "true" : "false") + 100));

      sprintf(rteJSON, "{\"name\":\"%s\",\"numPoints\":%d,\"len\":%.1f,\"loop\":%s}", name, numPoints, routeLen, loopState ? "true" : "false");

      return rteJSON;
}

char* routeListToJSON(const List *list) {

      char *rteListJSON;
      char **listArray;
      int i = 0;
      int strLength = 3;

      if(list == NULL || list->length == 0) {
            rteListJSON = malloc(sizeof(char) * 3);
            strcpy(rteListJSON, "[]");
            return rteListJSON;
      }

      rteListJSON = malloc(sizeof(char*) * 100);

      if(rteListJSON == NULL) {
            printf("ERROR! Failed malloc of rteListJSON (routeListToJSON).");
      }

      listArray = malloc(sizeof(char*) * list->length);

      if(listArray == NULL) {
            printf("ERROR! Failed malloc of listArray (routeListToJSON).");
      }

      Node *currNode;
      strcpy(rteListJSON, "[");
      currNode = list->head;
      while(currNode != NULL) {
            Route *currRte = (Route*)currNode->data;


            listArray[i] = routeToJSON(currRte);
            strLength = strLength + strlen(listArray[i]);
            strcat(rteListJSON, listArray[i]);

            rteListJSON = realloc(rteListJSON, sizeof(char*) * strLength);

            if(currNode->next == NULL)
                  break;

            strcat(rteListJSON, ",");

            currNode = currNode->next;
            i++;
      }
      strcat(rteListJSON, "]");

      for(i = 0; i < list->length; i++)
            free(listArray[i]);
      
      free(listArray);

      return rteListJSON;
}

char* trackListToJSON(const List *list) {

      char *trkListJSON;
      char **listArray;
      int strLength = 2;
      int i = 0;

      if(list == NULL || list->length == 0) {
            trkListJSON = malloc(sizeof(char) * 3);
            strcpy(trkListJSON, "[]");
            return trkListJSON;
      }

      trkListJSON = malloc(sizeof(char*) * 100);

      if(trkListJSON == NULL) {
            printf("ERROR! Failed malloc of trkListJSON (routeListToJSON).");
      }

      listArray = malloc(sizeof(char*) * list->length);

      if(listArray == NULL) {
            printf("ERROR! Failed malloc of listArray (routeListToJSON).");
      }

      Node *currNode;
      strcpy(trkListJSON, "[");
      currNode = list->head;
      while(currNode != NULL) {
            Track *currTrk = (Track*)currNode->data;

            listArray[i] = trackToJSON(currTrk);
            strLength = strLength + strlen(listArray[i]);
            strcat(trkListJSON, listArray[i]);

            if(currNode->next == NULL)
                  break;

            trkListJSON = realloc(trkListJSON, sizeof(char*) * strLength);

            strcat(trkListJSON, ",");

            currNode = currNode->next;
            i++;
      }
      strcat(trkListJSON, "]");

      for(i = 0; i < list->length; i++)
            free(listArray[i]);
      free(listArray);

      return trkListJSON;
}

char* GPXtoJSON(const GPXdoc* gpx) {

      char *gpxJSON;

      int numW;
      int numR;
      int numT;

      if(gpx == NULL) {
            gpxJSON = malloc(sizeof(char) * 3);
            strcpy(gpxJSON, "{}");
            return gpxJSON;
      }
      
      numW = getNumWaypoints(gpx);
      numR = getNumRoutes(gpx);
      numT = getNumTracks(gpx);

      gpxJSON = malloc(sizeof(char*) * (strlen(gpx->creator) + 100));

      sprintf(gpxJSON, "{\"version\":%.1f,\"creator\":\"%s\",\"numWaypoints\":%d,\"numRoutes\":%d,\"numTracks\":%d}", 
      gpx->version, gpx->creator, numW, numR, numT);

      return gpxJSON;
}

//* A3 FUNCTIONS

char *GPXFileLogJSON(char *fileName) {

      GPXdoc *gpx = NULL;
      char *gpxJSON = NULL;

      if(!(gpx = createValidGPXdoc(fileName, "./parser/bin/gpx.xsd"))) {
            printf("ERROR! Failed to create GPX JSON (GPXFileLogJSON).\n");
            return NULL;
      }

      if(!(gpxJSON = GPXtoJSON(gpx))) {
            printf("ERROR! NULL String from GPXtoJSON (GPXFileLogJSON).\n");
            return NULL;
      }

      deleteGPXdoc(gpx);
      //printf("Returning: %s\n", gpxJSON);

      return gpxJSON;
}

char *GPXViewPanelJSON(char *fileName) {

      
      GPXdoc *gpx = NULL;
      char *routeListJSON = NULL;
      char *trackListJSON = NULL;
      char *viewPanelJSON = NULL;

      if(!(gpx = createValidGPXdoc(fileName, "./parser/bin/gpx.xsd"))) {
            printf("ERROR! Failed to create GPX JSON (GPXFileLogJSON).\n");
            //deleteGPXdoc(gpx); don't do this
            return NULL;
      }

      if(!(routeListJSON = routeListToJSON(gpx->routes))) {
            printf("ERROR! NULL String from routeListToJSON (GPXViewPanelJSON).\n");
            return NULL;
      }

      if(!(trackListJSON = trackListToJSON(gpx->tracks))) {
            printf("ERROR! NULL String from routeListToJSON (GPXViewPanelJSON).\n");
            return NULL;
      }

      viewPanelJSON = malloc((sizeof(char) * strlen(routeListJSON) + 1) + (sizeof(char) * strlen(trackListJSON) + 100));

      strcpy(viewPanelJSON, "{\"routes\":");
      strcat(viewPanelJSON, routeListJSON);
      strcat(viewPanelJSON, ",\"tracks\":");
      strcat(viewPanelJSON, trackListJSON);
      strcat(viewPanelJSON, "}");

      free(routeListJSON);
      free(trackListJSON);

      deleteGPXdoc(gpx);
      printf("Returning: %s\n", viewPanelJSON);

      return viewPanelJSON;
}

char *getOtherDataRadio(char *fileName, char *dataName, int dataIndex) {
      GPXdoc *gpx = NULL;
      char *otherData = NULL;
      int i = 0;
      int strSize = 0;

      if(!(gpx = createValidGPXdoc(fileName, "./parser/bin/gpx.xsd"))) {
            printf("ERROR! Failed to create GPX JSON (getOtherDataRadio).\n");
            return NULL;
      }

      otherData = malloc(sizeof(char) * 100);

      if(strcmp(dataName, "Route") == 0) {

            Node *currRteNode = gpx->routes->head;
            Route *currRte = (Route*)gpx->routes->head->data;

            while(i != dataIndex) {
                  currRteNode = (Node*)currRteNode->next;
                  currRte = currRteNode->data;
                  i++;
            }

            //printf("currRte Name: %s\n", currRte->name);


            strcpy(otherData, "");

            Node *currNode = currRte->otherData->head;
            if(currNode == NULL) {
                  return NULL;
            }

            while(currNode != NULL) {
                  GPXData *currOtherData = currNode->data;
                  strSize = strSize + strlen(otherData) + strlen(currOtherData->name) + strlen(currOtherData->value) + 6;

                  otherData = realloc(otherData, strSize);
                  strcat(otherData, currOtherData->name);
                  strcat(otherData, ": ");
                  strcat(otherData, currOtherData->value);
                  strcat(otherData, "\n");

                  currNode = currNode->next;
            }

      }
      else if(strcmp(dataName, "Track") == 0) {
            Node *currTrkNode = gpx->tracks->head;
            Route *currTrk = (Route*)gpx->tracks->head->data;

            while(i != dataIndex) {
                  currTrkNode = (Node*)currTrkNode->next;
                  currTrk = currTrkNode->data;
                  i++;
            }
            //printf("currTrk Name: %s\n", currTrk->name);

            strcpy(otherData, "");

            Node *currNode2 = currTrk->otherData->head;
            if(currNode2 == NULL) {
                  return NULL;
            }

            while(currNode2 != NULL) {
                  GPXData *currOtherData2 = currNode2->data;
                  strSize = strSize + strlen(otherData) + strlen(currOtherData2->name) + strlen(currOtherData2->value) + 6;

                  otherData = realloc(otherData, strSize);
                  strcat(otherData, currOtherData2->name);
                  strcat(otherData, ": ");
                  strcat(otherData, currOtherData2->value);
                  strcat(otherData, "\n");

                  currNode2 = currNode2->next;
            }
            
      }

      deleteGPXdoc(gpx);
      printf("Returning: \n%s\n", otherData);
      
      return otherData;
}

int renameRouteTrack(char *fileName, char *dataName, int dataIndex, char* newName) {
      GPXdoc *gpx = NULL;
      int i = 0;

      if(!(gpx = createValidGPXdoc(fileName, "./parser/bin/gpx.xsd"))) {
            printf("ERROR! Failed to create GPX JSON (renameRouteTrack).\n");
            //deleteGPXdoc(gpx); //*don't do this
            return 0;
      }

      if(strcmp(dataName, "Route") == 0) {

            Node *currRteNode = gpx->routes->head;
            Route *currRte = (Route*)gpx->routes->head->data;

            //*FIND THE ROUTE AT INDEX
            while(i != dataIndex) {
                  currRteNode = (Node*)currRteNode->next;
                  currRte = currRteNode->data;
                  i++;
            }

            if(currRte->name != NULL) {
                  printf("Renaming %s.\n", currRte->name);
                  currRte->name = realloc(currRte->name, sizeof(char) * strlen(newName) + 1);
                  strcpy(currRte->name, newName);
                  printf("Renamed to %s.\n", currRte->name);
                  
                  if(validateGPXDoc(gpx, "./parser/bin/gpx.xsd")) {
                        printf("%s successfully validated.\n", fileName);

                        if(writeGPXdoc(gpx, fileName)) {
                              printf("%s successfully written.\n", fileName);
                        }
                        else {
                              deleteGPXdoc(gpx);
                              return 0;
                        }
                  }
                  else {
                        deleteGPXdoc(gpx);
                        return 0;
                  }
            }

      }
      else if(strcmp(dataName, "Track") == 0) {
            Node *currTrkNode = gpx->tracks->head;
            Route *currTrk = (Route*)gpx->tracks->head->data;

            //*FIND THE TRACK AT INDEX
            while(i != dataIndex) {
                  currTrkNode = (Node*)currTrkNode->next;
                  currTrk = currTrkNode->data;
                  i++;
            }

            if(currTrk->name != NULL) {
                  printf("Renaming %s.\n", currTrk->name);
                  currTrk->name = realloc(currTrk->name, sizeof(char) * strlen(newName) + 1);
                  strcpy(currTrk->name, newName);
                  printf("Renamed to %s.\n", currTrk->name);

                  if(validateGPXDoc(gpx, "./parser/bin/gpx.xsd")) {
                        printf("%s successfully validated.\n", fileName);

                        if(writeGPXdoc(gpx, fileName)) {
                              printf("%s successfully written.\n", fileName);
                        }
                        else {
                              deleteGPXdoc(gpx);
                              return 0;
                        }
                  }
                  else {
                        deleteGPXdoc(gpx);
                        return 0;
                  }

            }
            //printf("currTrk Name: %s\n", currTrk->name);
      }
      deleteGPXdoc(gpx);
      return 1;
}

char* getRouteAtIndex(char *fileName, int dataIndex) {
      GPXdoc *gpx = NULL;
      char *rteName = NULL;
      int i = 0;

      if(!(gpx = createValidGPXdoc(fileName, "./parser/bin/gpx.xsd"))) {
            printf("ERROR! Failed to create GPX JSON (renameRouteTrack).\n");
            //deleteGPXdoc(gpx); //*don't do this
            return NULL;
      }


      Node *currRteNode = gpx->routes->head;
      Route *currRte = (Route*)gpx->routes->head->data;

      //*FIND THE ROUTE AT INDEX
      while(i != dataIndex) {
            currRteNode = (Node*)currRteNode->next;
            currRte = currRteNode->data;
            i++;
      }

      if(currRte->name != NULL) {
            rteName = malloc(sizeof(char) * strlen(currRte->name));
            strcpy(rteName, currRte->name);
            printf("rteName: %s\n", rteName);
      }

      deleteGPXdoc(gpx);

      return rteName;
}

char *getPathJSON(char *fileName, float p1_lat, float p1_lon, float p2_lat, float p2_lon, float delta) {
      
      GPXdoc *gpx = NULL;
      int strsize = 0;
      int rteCount = 0;
      int trkCount = 0;

      char *pathsJSON = NULL;

      pathsJSON = malloc(sizeof(char) * 100);

      strcpy(pathsJSON, "{\"routes\":");

      if(!(gpx = createValidGPXdoc(fileName, "./parser/bin/gpx.xsd"))) {
            printf("ERROR! Failed to create GPX JSON (GPXFileLogJSON).\n");
            free(pathsJSON);
            return NULL;
      }

      List *rteList = getRoutesBetween(gpx, p1_lat, p1_lon, p2_lat, p2_lon, delta);

      if(rteList != NULL) {
            char *rteListJSON = routeListToJSON(rteList);
            strsize = strsize + strlen(pathsJSON) + strlen(rteListJSON) + 25;
            pathsJSON = realloc(pathsJSON, (sizeof(char) * strsize));
            strcat(pathsJSON, rteListJSON);
            rteCount++;

            strcat(pathsJSON, ",");
      }

      strcat(pathsJSON, "\"tracks\":");

      List *trkList = getTracksBetween(gpx, p1_lat, p1_lon, p2_lat, p2_lon, delta);

      if(trkList != NULL) {

            char *trkListJSON = trackListToJSON(trkList);
            strsize = strsize + strlen(pathsJSON) + strlen(trkListJSON) + 25;
            pathsJSON = realloc(pathsJSON, (sizeof(char) * strsize));
            strcat(pathsJSON, trkListJSON);
            trkCount++;

            strcat(pathsJSON, ",");
      }
      else {
            strcat(pathsJSON, "[]");
      }
      strcat(pathsJSON, "}");
      //strcat(pathsJSON, "}");

      if(trkCount == 0 && rteCount == 0) {
            free(pathsJSON);
            deleteGPXdoc(gpx);
            return NULL;
      }

      deleteGPXdoc(gpx);
      return pathsJSON;
}

int createGPXFile(char *fileName, char *creator) {

      GPXdoc *gpx = NULL;
      List   *wayp = NULL, *routes = NULL, *tracks = NULL;

      gpx = malloc(sizeof(GPXdoc));
      gpx->creator = malloc(sizeof(char) * 256);

      strcpy(gpx->creator, creator);
      strcpy(gpx->namespace, "http://www.topografix.com/GPX/1/1"); //default namespace
      gpx->version = 1.1;                                          //default version

      wayp = initializeList(waypointToString, deleteWaypoint, compareWaypoints);
      routes = initializeList(routeToString, deleteRoute, compareRoutes);
      tracks = initializeList(trackToString, deleteTrack, compareTracks);

      gpx->waypoints = wayp;
      gpx->routes = routes;
      gpx->tracks = tracks;

      if(validateGPXDoc(gpx, "./parser/bin/gpx.xsd")) {
            if(writeGPXdoc(gpx, fileName)) {
                  printf("GPX File: %s successfully created.\n", fileName);
                  deleteGPXdoc(gpx);
                  return 1;
            }
            else {
                  deleteGPXdoc(gpx);
                  return 0;
            }
      }
      else {
            deleteGPXdoc(gpx);
            return 0;
      }

      return 0;
      
}

int addRouteToFile(char *fileName, char* routeName) {

      GPXdoc *gpx = NULL;

      if(!(gpx = createValidGPXdoc(fileName, "./parser/bin/gpx.xsd"))) {
            printf("ERROR! Failed to add route, invalid gpx file. (addRouteToFile).\n");
            //deleteGPXdoc(gpx); don't do this
            return 0;
      }

      Route *toAdd = malloc(sizeof(Route));

      toAdd->name = malloc(sizeof(char) * strlen(routeName));
      strcpy(toAdd->name, routeName);

      toAdd->waypoints = initializeList(waypointToString, deleteWaypoint, compareWaypoints);
      toAdd->otherData = initializeList(gpxDataToString, deleteGpxData, compareGpxData);

      addRoute(gpx, toAdd);

      if(validateGPXDoc(gpx, "./parser/bin/gpx.xsd")) {
            if(writeGPXdoc(gpx, fileName)) {
                  printf("GPX File: %s successfully created. (addRouteToFile)\n", fileName);
                  deleteGPXdoc(gpx);
                  return 1;
            }
            else {
                  deleteGPXdoc(gpx);
                  return 0;
            }
      }
      else {
            deleteGPXdoc(gpx);
            return 0;
      }

      deleteGPXdoc(gpx);
      return 0;
}

int addWaypointToFile(char *fileName, char *dataName, int dataIndex, char* wName, double wLat, double wLon) {

      GPXdoc *gpx = NULL;
      int i = 0;

      // if(!(gpx = createValidGPXdoc(fileName, "./parser/bin/gpx.xsd"))) {
      //       printf("ERROR! Failed to create GPX JSON (GPXFileLogJSON).\n");
      //       //deleteGPXdoc(gpx); //*don't do this
      //       return NULL;
      // }

      if(!(gpx = createValidGPXdoc(fileName, "./parser/bin/gpx.xsd"))) {
            printf("ERROR! Failed to create GPX JSON (GPXFileLogJSON).\n");
            //deleteGPXdoc(gpx); don't do this
            return 0;
      }

      Waypoint *toAdd = NULL;

      toAdd = malloc(sizeof(Waypoint));
      toAdd->name = malloc(sizeof(char) * strlen(wName));
      
      strcpy(toAdd->name, wName);
      toAdd->latitude = wLat;
      toAdd->longitude = wLon;
      toAdd->otherData = initializeList(gpxDataToString, deleteGpxData, compareGpxData);

      if(strcmp(dataName, "Route") == 0) {

            Node *currRteNode = gpx->routes->head;
            Route *currRte = (Route*)gpx->routes->head->data;

            //*FIND THE ROUTE AT INDEX
            while(i != dataIndex) {
                  currRteNode = (Node*)currRteNode->next;
                  currRte = currRteNode->data;
                  i++;
            }

            addWaypoint(currRte, toAdd);

            if(validateGPXDoc(gpx, "./parser/bin/gpx.xsd")) {
                  if(writeGPXdoc(gpx, fileName)) {
                        printf("GPX File: %s successfully created. (addWaypointToFile)\n", fileName);
                        deleteGPXdoc(gpx);
                        return 1;
                  }
                  else {
                        deleteGPXdoc(gpx);
                        return 0;
                  }
            }
            else {
                  deleteGPXdoc(gpx);
                  return 0;
            }


      }

      deleteGPXdoc(gpx);
      return 1;
}

char* waypToJSON(const Waypoint *wayp) {

      char* waypJSON;
      char* name;

      if(wayp == NULL) {
            return "{}";
      }
      
      if(strcmp(wayp->name, " ") != 0)
            name = wayp->name;
      else
            name = "None";

      waypJSON = malloc(sizeof(char*) * (strlen(name) + 100));

      sprintf(waypJSON, "{\"name\":\"%s\",\"lat\":%.6lf,\"lon\":%.6lf}", wayp->name, wayp->latitude, wayp->longitude);

      return waypJSON;
}

char *rteWaypToJSON(char *fileName, char *rteName) {

      GPXdoc *gpx = NULL;
      char *rteWaypJSON = NULL;
      int strSize = 0;

      if(!(gpx = createValidGPXdoc(fileName, "./parser/bin/gpx.xsd"))) {
            printf("ERROR! Failed to create GPX JSON (GPXFileLogJSON).\n");
            //deleteGPXdoc(gpx); //*don't do this
            return NULL;
      }

      // if(!(gpx = createValidGPXdoc(fileName, "./gpx.xsd"))) {
      //       printf("ERROR! Failed to create GPX doc (rteWaypToJSON).\n");
      //       //deleteGPXdoc(gpx); don't do this
      //       return NULL;
      // }

      if(gpx->routes->length == 0) {
            return NULL;
      }


      Node *currNode;
      
      currNode = gpx->routes->head;
      while(currNode != NULL) {
            Route *currRte = (Route*)currNode->data;

            if(strcmp(currRte->name, rteName) == 0) {

                  if(currRte->waypoints->length == 0) {
                        break;
                  }

                  rteWaypJSON = malloc(sizeof(char) * 100);
                  strcpy(rteWaypJSON, "");
                  strcat(rteWaypJSON, "{\"waypoints\":[");
                  strSize = strSize + strlen(rteWaypJSON);

                  Node *waypNode;
                  waypNode = currRte->waypoints->head;

                  while(waypNode != NULL) {
                        Waypoint *currWayp = (Waypoint*)waypNode->data;
                        char *tmpWaypJSON = waypToJSON(currWayp);
                        strSize = strSize + strlen(tmpWaypJSON);

                        //*GET WAYPOINT STRING
                        rteWaypJSON = realloc(rteWaypJSON, (sizeof(char) * strSize + 50));
                        strcat(rteWaypJSON, tmpWaypJSON);
                        free(tmpWaypJSON);
                        
                        if(waypNode->next != NULL) {
                              strcat(rteWaypJSON, ",");
                        }
                        waypNode = waypNode->next;
                  }

                  strcat(rteWaypJSON, "]}");
                  break;
            }

            if(currNode->next == NULL) break;

            currNode = currNode->next;
      }


      return rteWaypJSON;
}


void addWaypoint(Route *rt, Waypoint *pt) {

      if(rt == NULL) {
            printf("ERROR! NULL Route (addWaypoint).\n");
            return;
      }

      if(rt->waypoints == NULL) {
            rt->waypoints = initializeList(waypointToString, deleteWaypoint, compareWaypoints);
      }

      insertBack(rt->waypoints, (Waypoint*)pt);
}

void addRoute(GPXdoc* doc, Route* rt) {
      
      if(doc == NULL) {
            printf("ERROR! NULL GPXdoc (addRoute).\n");
      }

      if(doc->routes == NULL) {
            doc->routes = initializeList(routeToString, deleteRoute, compareRoutes);
      }

      insertBack(doc->routes, (Route*)rt);
}


//didn't end up needing these
GPXdoc* JSONtoGPX(const char* gpxString) {

      GPXdoc* jsonGPX = NULL;


      return jsonGPX;
}

Waypoint* JSONtoWaypoint(const char* gpxString) {

      Waypoint *jsonWayp = NULL;

      return jsonWayp;
}

Route* JSONtoRoute(const char* gpxString) {

      Route *jsonRte = NULL;


      return jsonRte;
}