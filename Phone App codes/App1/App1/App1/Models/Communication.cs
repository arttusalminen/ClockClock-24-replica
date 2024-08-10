using System;
using System.Collections.Generic;
using System.Net;
using System.Text;

namespace App1.Models
{
    public static class Communication
    {

        public static bool PostData(string uri, int timeout, out string toDisplay)
        {
            HttpWebRequest httpWebRequest = (HttpWebRequest)HttpWebRequest.Create(uri);
            httpWebRequest.Method = "POST";
            httpWebRequest.Timeout = timeout;
            try
            {
                HttpWebResponse response = (HttpWebResponse)httpWebRequest.GetResponse();

                // Check if sending time was successful:
                if (response.StatusCode != HttpStatusCode.Accepted)
                {
                    toDisplay = "Updating data failed.";
                    return false;
                }
                else
                {
                    toDisplay = "Success!";
                    return true;
                }
            }
            catch (Exception e)
            {
                toDisplay = String.Format("Updating time failed: {0}", e.Message == null ? "NULL" : e.Message);
                return false;
            }
        }

    }
}
