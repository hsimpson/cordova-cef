/*jslint browser: true, devel: true */

var app = (function () {
    'use strict';
    
    function onDeviceReady() {
        // Used by unit tests to tell when the page is loaded.
        window.pageIsLoaded = true;
        // do your thing!
        console.log("Cordova deviceready");
    }
    
    function onPause() {
        console.log("Cordova pause");
    }
    
    function onResume() {
        console.log("Cordova resume");
    }
    
    return {
        initialize : function () {
            document.addEventListener("deviceready", onDeviceReady, false);
            document.addEventListener("pause", onPause, false);
            document.addEventListener("resume", onResume, false);
        }
    };
}());


