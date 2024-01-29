// service-worker.js

let nativeHostPort;
let tabId;

self.addEventListener('install', event => {
  event.waitUntil(
    establishNativeHostConnection()
      .then(() => {
        console.log('Native host connection established during installation')
        // Send a ping event as soon as the connection is established
        sendPingEvent();
      })
      .catch(error => console.error('Error establishing native host connection during installation:', error))
  );
});

self.addEventListener('activate', event => {
  // Perform activation tasks if needed
});


async function establishNativeHostConnection() {
  if (nativeHostPort && nativeHostPort.sender.tab) {
    // Resolve immediately if already connected
    return;
  }

  return new Promise((resolve, reject) => {
    // Attempt to connect to the native messaging host
    nativeHostPort = chrome.runtime.connectNative('com.chromecast.nativehost.cpp');

    // Listen for messages from the native messaging host
    nativeHostPort.onMessage.addListener(async msg => {

      if (msg.request === 'tabInfo') {
        // Forward the message to the content script of the current tab
        tabInfoResponse = null
        try {
          // Get the response from getTabInfoRequest
          tabInfoResponse = await getTabInfoRequest();
        } catch (error) {
          tabInfoResponse = {error : "fail to message content script"}
        }

        // Send the response back to the native host
        try {
          await new Promise((resolve, reject) => {
            nativeHostPort.postMessage({ response: 'tabInfo', data: tabInfoResponse }, () => {
              if (chrome.runtime.lastError) {
                reject(new Error(chrome.runtime.lastError.message));
              } else {
                resolve();
              }
            });
          });
        } catch (error) {
          console.error('Error sending response to native host:', error);
        }
      }
    });

    // Handle disconnection
    nativeHostPort.onDisconnect.addListener(async () => {
      console.error('Native host disconnected. Attempting to reconnect...');
      nativeHostPort = null;
      await establishNativeHostConnection();
      console.log('Reconnected to the native host');
    });

    // Resolve when the connection is established
    nativeHostPort.onDisconnect.addListener(() => {
      resolve();
    });

    // Reject if there is an error during connection
    nativeHostPort.onDisconnect.addListener(() => {
      reject(new Error('Failed to establish native host connection'));
    });
  });
}

const siteMap = {  
  "www.youtube.com": "youtube",  
  "www.netflix.com": "netflix"  
};  

function getTag(url){  
  const host = new URL(url).hostname;
  return siteMap[host] || "og";
}  

async function getTabInfoRequest() {
  return new Promise((resolve, reject) => {
    chrome.tabs.query({ active: true, lastFocusedWindow: true }, async tabs => {
      if (tabs && tabs.length > 0) {
          try {
              let url = tabs[0].url
              let tag = getTag(url)
              sendMessageToTab(tabs[0].id, { request: "meta", tag })
              .then(response => {
                if (response) {
                  resolve({ url, meta: response });
                } else {
                  resolve({ url });
                }
              })
          } catch(error) {
            resolve({url:tabs[0].url})
          }
      } else {
        resolve({error: "No active tabs found"})
      }
    });
  });
}

function sendMessageToTab(tabId, message) {
  return new Promise((resolve, reject) => {
    chrome.tabs.sendMessage(tabId, message, null, response => {
      if (chrome.runtime.lastError) {
        reject(chrome.runtime.lastError);
      } else {
        resolve(response);
      }
    });
  });
}
