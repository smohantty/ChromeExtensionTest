// service-worker.js

let nativeHostPort;
let tabId;

self.addEventListener('install', event => {
  event.waitUntil(
    establishNativeHostConnection()
      .then(() => console.log('Native host connection established during installation'))
      .catch(error => console.error('Error establishing native host connection during installation:', error))
  );
});

self.addEventListener('activate', event => {
  // Perform activation tasks if needed
});

self.addEventListener('message', async event => {
  if (event.data.message === 'getTabInfo') {
    try {
      await sendTabInfoRequest();
    } catch (error) {
      console.error('Error sending message to native host:', error);
      // If there's an error, attempt to reconnect
      await establishNativeHostConnection();
    }
  }
});

async function establishNativeHostConnection() {
  if (nativeHostPort && nativeHostPort.sender.tab) {
    // Resolve immediately if already connected
    return;
  }

  return new Promise((resolve, reject) => {
    // Attempt to connect to the native messaging host
    nativeHostPort = chrome.runtime.connectNative('com.suv.service');

    // Listen for messages from the native messaging host
    nativeHostPort.onMessage.addListener(async msg => {
      console.log('Message from native host:', msg);

      if (msg.action === 'forwardToContentScript') {
        // Forward the message to the content script of the current tab
        try {
          await sendTabInfoRequest();
        } catch (error) {
          console.error('Error sending message to content script:', error);
        }

        // Send the response back to the native host
        try {
          await new Promise((resolve, reject) => {
            nativeHostPort.postMessage({ action: 'forwardResponse', data: response }, () => {
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

async function sendTabInfoRequest() {
  return new Promise((resolve, reject) => {
    chrome.tabs.query({ active: true, currentWindow: true }, async tabs => {
      if (tabs && tabs.length > 0) {
        tabId = tabs[0].id;

        // Send the message to the content script
        try {
          const response = await new Promise((resolve, reject) => {
            chrome.tabs.sendMessage(tabId, { action: 'getTabInfo' }, response => {
              if (chrome.runtime.lastError) {
                reject(new Error(chrome.runtime.lastError.message));
              } else {
                resolve(response);
              }
            });
          });
          resolve(response);
        } catch (error) {
          reject(error);
        }
      } else {
        reject(new Error('Error querying tabs: No active tabs found.'));
      }
    });
  });
}
