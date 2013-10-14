function onBodyLoad() {		
    document.addEventListener("deviceready", onDeviceReady, false);
    document.addEventListener("pause", yourCallbackFunction, false);
}


function onDeviceReady()
{
    // Used by unit tests to tell when the page is loaded.
    window.pageIsLoaded = true;
    // do your thing!
    console.log("Cordova is working")
}

function onPause()
{
    
}