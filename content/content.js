 // content.js

chrome.runtime.onMessage.addListener((msg, sender, sendResponse) => {
  if (msg.request === 'meta') {
      // Perform asynchronous operation (e.g., fetching data)
      fetchMeta(msg.tag)
        .then(meta => {
          // Send the response once the asynchronous operation is complete
          sendResponse(meta);
        })
        .catch(error => {
          // Handle errors and send an empty error object as a response
          sendResponse({});
        });

      // Return true to indicate that the response will be sent asynchronously
      return true;
  }
});

// Example asynchronous operation function
function fetchMeta(tag) {
  return new Promise((resolve, reject) => {
    // Simulate an asynchronous operation (e.g., fetching data)
    setTimeout(() => {
      // Resolve the Promise with the result (e.g., meta data)
      const meta = getMeta(tag);
      resolve(meta);
    }, 10); // Simulated delay of 10ms
  });
}


function getMeta(tag) {
   switch (tag.toString()) {  
       case "youtube":  
         return getYoutubeMeta()
         break;  
       case "netflix":
           return {type:tag}
         break;  
       case "og":  
           return getOgMeta()
         break;
       default:  
           return {tag}
   }
}


function getOgMeta() {
   var json = {}
   json['type'] = 'og'
   try {
       getMetaProperty('og:title', json, 'title')
       getMetaProperty('og:description', json, 'description')
       getMetaProperty('og:image', json, 'thumbnailUrl')
       getMetaProperty('og:type', json, 'genre')
       getMetaProperty('og:article:author', json, 'author')
       getMetaProperty('og:url', json, 'url')
   } catch(error) {
      json['error'] = error
   }
   return json
}


function getYoutubeMeta() {
   var json = {}
   json['type'] = 'yt'
   getMetaTag('title', json, 'title')
   getMetaTag('description', json, "description")
   getMetaItemProp('uploadDate', json, 'upload_date')
   getMetaItemProp('genre', json, 'genre')
   getLinkProp('thumbnailUrl', json, 'thumbnailUrl')
   getTimeStamp(json, 'timestamp')
   return json
}

function getMetaTag(id, json, key) {
   var metaTag = document.querySelector('meta[name=' + id + ']');
   if (metaTag) {
       json[key] = metaTag.getAttribute('content')
   }
}

function getMetaProperty(id, json, key) {
  var metaProperty = document.querySelector("meta[property='" + id + "']");  
  if (metaProperty) {
      json[key] = metaProperty.getAttribute('content')
  }
}


function getMetaItemProp(id, json, key) {
   var metaProp = document.querySelector('meta[itemprop=' + id + ']');  
   if (metaProp) {
       json[key] = metaProp.getAttribute('content')
   }
}

function getLinkProp(id, json, key) {
   var linkProp = document.querySelector('link[itemprop=' + id + ']');
   if (linkProp) {
       json[key] = linkProp.getAttribute('href')
   }
}

function getTimeStamp(json, key) {
   var progressBar = document.querySelector('.ytp-progress-bar');
   if (progressBar) {
       json[key] = progressBar.getAttribute('aria-valuenow'); 
   }
}
  
  