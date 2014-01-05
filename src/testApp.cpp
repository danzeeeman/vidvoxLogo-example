#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup()
{
    
    //Set the background to my favorite RGB black
    ofBackground( 4 , 5 , 6 ) ;
    //Set the Frame Rate
    ofSetFrameRate(60);
    //Enbal
    ofSetVerticalSync(true);
    
    //
    //  Assignment Number 3
    //  1) Add two new value to the Application's XML File, windowWidth and windowHeight
    //  2) Set the dimensions of the app's window to the values from the XML file
    //      reference the ofAppRunner class for more information
    //        http://www.openframeworks.cc/documentation/application/ofAppRunner.html
    //
    //  3) Set the Window's Title to the Sphyon Server Name from Assignment 2
    //  
    
    //Load your app settings file into an ofxXmlSettings object
    settings.load(ofToDataPath("settings.xml"));
    
    //Here weImage Settings
   
    fileName = settings.getValue("fileName", "vidvox1-01.png");//<fileName>foobar</fileName>
    imageWidth = settings.getValue("width", 800);//<width>400</width>
    imageHeight = settings.getValue("height", 400);//<height>200</height>
    
    
    //Load OSC Settings
    
    oscForceRadius = settings.getValue("forceRadius", "/FromVDMX/forceRadius");//<forceRadius>/FromVDMX/forceRadius</forceRadius>
    oscFriction = settings.getValue("friction", "/FromVDMX/friction");//<friction>/FromVDMX/forceRadius</friction>
    oscSpringEnabled = settings.getValue("springEnabled", "/FromVDMX/springEnabled");//<springEnabled>/FromVDMX/forceRadius</springEnabled>
    oscSpringFactor = settings.getValue("springFactor", "/FromVDMX/springFactor");//<springFactor>/FromVDMX/forceRadius</springFactor>
    oscMode = settings.getValue("mode", "/FromVDMX/mode");
    
    
    //we setup our OSC receiver
    receiver.setup(settings.getValue("port", 12345));
    
    //load our image inside bin/data
    image.loadImage ( fileName ) ;
    
    
    //
    //  Assignment Number 1
    //  1) Read the desired imageWidth and imageHeight from the XML file
    //  2) Read the size of the ofImage
    //  3) Auto scale the ofImage to fit within the desired height and width
    //
    //  remember desiredWidth = (acutalWidth * desiredHeight) / acutalHeight
    //
    
    image.resize(imageWidth, imageHeight);
    
    //if the app performs slowly raise this number
    sampling = 1 ;
    
    //Retrieve the pixels from the loaded image
    unsigned char * pixels = image.getPixels() ;
    //store width and height for optimization and clarity
    int w = image.width ; 
    int h = image.height ; 
    
    //offsets to center the particle son screen
    int xOffset = (ofGetWidth() - w ) /2 ; 
    int yOffset = (ofGetHeight() - h ) /2 ;
    
    //Loop through all the rows
    for ( int x = 0 ; x < w ; x+=sampling ) 
    {
        //Loop through all the columns
        for ( int y = 0 ; y < h ; y+=sampling ) 
        {
            //Pixels are stored as unsigned char ( 0 <-> 255 ) as RGB
            //If our image had transparency it would be 4 for RGBA
            int index = ( y * w + x ) * 4 ;
            ofColor color ; 
            color.r = pixels[index] ;       //red pixel
            color.g = pixels[index+1] ;     //blue pixel
            color.b = pixels[index+2] ;     //green pixel
            if(pixels[index+3] != 0){
                particles.push_back( Particle ( ofPoint ( x + xOffset, y + yOffset) , color ) ) ;
            }
        }
    }
    
    ofSetFrameRate( 60 ) ;
    numParticles = particles.size() ;
    
    //Set default spring and sink values
    cursorMode = true ;
    forceRadius = 1;
    friction = 0.85 ; 
    springFactor = 0.12 ; 
    springEnabled = true ;
    
    // This is out syphon server
    // We can set its name to any String
    // It's usally wise to uniquely name each instance
    
    
    //
    //  Assignment Number 2
    //  1) Add a Syphon Server Name value to the Application's XML File
    //  2) Read the newly created value from the XML File
    //  3) Set the Syphon Server name to the value from the XML
    //
    
    
    server.setName("vidvoxLogo");
}

//--------------------------------------------------------------
void testApp::update(){
    
    ofPoint diff ;          //Difference between particle current position and particle's spawn position
    float dist ;            //distance from particle to the spawn point ( as the crow flies ) 
    float ratio ;           //ratio of how strong the effect is = 1 + (-dist/maxDistance) ;
    const ofPoint home = ofPoint(ofGetWindowWidth()/2, ofGetWindowHeight()/2);

    
    //
    // While we have OSC messages lets process them
    //
    while(receiver.hasWaitingMessages()){
        ofxOscMessage b;
        receiver.getNextMessage(&b);
        cout<<b.getAddress()<<endl;
      

        if(b.getAddress() == oscForceRadius){
            
            //
            // If it is a ForceRadius message we read the the first value '0' as a float.
            // You can configure VMDX to send the value of a slider as a normalized float value via OSC.  Normalized means it is between 0...1.
            // We read the value VDMX and Map it between 0 and either the WindowWidth or the WindowHeight, which ever is larges
            //
            
            forceRadius = ofMap(b.getArgAsFloat(0), 0, 1, 0, ofGetWindowWidth()>ofGetWindowHeight()?ofGetWindowWidth():ofGetWindowHeight());
        }else if(b.getAddress() == oscFriction){
            
            //
            // If it is a oscFriction message we read the the first value '0' as a float.
            // The friction of the system should always be between 0 and 1.
            //
            
            friction = b.getArgAsFloat(0);
        }else if(b.getAddress() == oscSpringEnabled){
            
            //
            // If it is a oscSpringEnabled message we read the the first value '0' as a Integer.
            // You can configure VMDX to send a button click as an Integer value.
            // Here we turn that into a boolean value springEnabled
            // if the value is High, springEnabled = true, if the value is low, springEnabled = false
            //

            if(b.getArgAsInt32(0) == 0){
                springEnabled = false;
            }else{
                springEnabled = true;
            }
        }else if(b.getAddress() == oscSpringFactor){
            
            //
            // If it is a oscSpringFactor message we read the the first value '0' as a float.
            // The springFactor of the system should always be between 0 and 1
            springFactor = b.getArgAsFloat(0);
        
        }else if(b.getAddress() == oscMode){
            //
            // If it is a oscMode message we read the the first value '0' as a Integer.
            // Here we turn that into a boolean value cursorMode
            // if the value is High, cursorMode = true, if the value is low, cursorMode = false
            // 
            
            if(b.getArgAsInt32(0) == 0){
                cursorMode = false;
            }else{
                cursorMode = true;
            }
        }
        
    }
    

    //Create an iterator to cycle through the vector
    std::vector<Particle>::iterator p ; 
    for ( p = particles.begin() ; p != particles.end() ; p++ ) 
    {
        //
        //  Assignment Number 4
        //  1) Add an OSC Address called oscRatio to the XML file
        //  2) Add a new OSC receiver to the OSC while loop
        //  3) Read this value and map it to a value between 0 and 100
        //  4) Set this value as the ratio of how strong the spring effect is
        //  5) Configure VDMX to send a new OSC message to your App
        //  6) Play with mapping the Normalized values from VDMX (part 3)
        //
        ratio = 2.0f ;
        p->velocity *= friction ; 
        
        //reset acceleration every frame
        
        p->acceleration = ofPoint() ;
        
        diff = p->spawnPoint - p->position ;
        
        dist = ofDist( 0 , 0 , diff.x , diff.y ) ;
 
        // If the distance between the particles current position and the spawnPoint is 0
        // then we need to shake things up a by settings its position to a random point
        

        if(dist == 0){
            p->position = ofPoint(ofRandom(p->spawnPoint.x-1, p->spawnPoint.x+1), ofRandom(p->spawnPoint.y-1, p->spawnPoint.y+1));
             dist = ofDist( 0 , 0 , diff.x , diff.y ) ;
        }
        //  If the partcle are within the zone of interaction
        //  We apply either a positive force or negitive force
        if ( dist < forceRadius )
        {
            ratio = -1 + dist / forceRadius ; 
            if ( cursorMode ) 
                p->acceleration -= ( diff * ratio) ;
            else
                p->acceleration += ( diff * ratio ) ; 
        }
        
        
        if ( springEnabled ) 
        {
            //Move back to the original position
            p->acceleration.x += springFactor * (p->spawnPoint.x - p->position.x);
            p->acceleration.y += springFactor * (p->spawnPoint.y - p->position.y) ;
        }
        
        //we update the velocity of the particle
        //we update the position of the particle based on its velocity

        p->velocity += p->acceleration * ratio ; 
        p->position += p->velocity ; 
    }
}

//--------------------------------------------------------------
void testApp::draw() {
    //
    // We clear the COLOR buffer and the DEPTH buffer
    // This is a very important step to receive clean images via Syphon
    //
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //Begin the openGL Drawing Mode
    
    glBegin(GL_POINTS);
    
    //Lines look Cool
    //glBegin(GL_LINE_STRIP);
    
    //Triangles look Cool too 
    //glBegin(GL_TRIANGLES);

    //Create an iterator to cycle through the vector
    std::vector<Particle>::iterator p ;
    for ( p = particles.begin() ; p != particles.end() ; p++ )
    {
        // Read the Stored color from each particle
        glColor3ub((unsigned char)p->color.r,(unsigned char)p->color.g,(unsigned char)p->color.b);
        // Draw the particle
        
        //
        //  Assignment Number 5
        //  1) Shake things up a bit on the Z axis
        //  
        
        glVertex3f(p->position.x, p->position.y, 0 );
    }
    //End openGL
    glEnd();
    

    ofSetColor ( 255 , 255 , 255 ) ;
    // We Enable AlphaBlending
    ofEnableAlphaBlending();
    // We publish the Screen to Syphon 
    server.publishScreen();
    
    
    // Anything draw to the screen after this point will not be in the syphon stream
    
    
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
   
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

