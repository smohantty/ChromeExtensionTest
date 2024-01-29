Protocol:
Native Extension sends Json request Object and receives Json response object.
Below are the JSON format , the response object changes depening on the content in active tab.


Request Obj
===========
{
    "request":"tabInfo"
}


Response Obj
=============

==> For Empty Tab / Chrome Tab

{
    "response":"tabInfo",
    "data":{
        "error":"empty tab"
    }
}

==> For Local File
{
    "response":"tabInfo",
    "data":{
        "file":"/home/suv/document.txt"
    }
}

==> For OpenGraph meta info

{
    "response":"tabInfo",
    "data":{
        "url":"https://www.ndtv.com/india-news/adani-flagship-stock-may-return-over-50-says-analyst-4952495#pfrom=home-ndtv_topstories","meta":{
            "type":"og",
            "title":"Adani Flagship Stock May Retufirm is central to India's economic ambitions.",
            "thumbnailUrl":"https://c.ndtvimg.com/2024-01/a9rq3it8_adani-generic-bloomberg_625x300_29_January_24.jpeg?ver-20240117.06","genre":"article",
            "url":"https://www.ndtv.com/india-news/adani-flagship-stock-may-return-over-50-says-analyst-4952495"
        }
    }
}

==> For Youtube
{
    "response":"tabInfo",
    "data":{
        "url":"https://www.youtube.com/watch?v=EOQB8PTLkpY",
        "meta":{
            "type":"yt",
            "description":"Enjoy the videos and music you love, upload original content, and share it all with friends, family, and the world",
            "timestamp":"16"
            }
        }
}


==> For Netflix

{
    "response":"tabInfo",
    "data":{
        "url":"https://www.netflix.com/watch/81572819?trackId=14170287",
        "meta":{
            "type":"netflix"
        }
    }
}

TODO: Need to fix the meta issue for Netflix.
Note: Currently not able to extract Netflix meta info from Extension , so fallback method is extract it in Android by downloading the URL again.


