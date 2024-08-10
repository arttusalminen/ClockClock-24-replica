using App1.Models;
using System;
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Net.Http;
using System.Text;
using System.Threading.Tasks;
using Xamarin.Forms;

namespace Stuff.Models
{
    public class Clock
    {
        private static readonly Lazy<Clock> _clock = new Lazy<Clock>(() => new Clock());
        private Clock() { }
        public static Clock Instance
        {
            get { return _clock.Value; }
        }

        private DateTime lastUpdate;

        public Task<bool> UpdateClockTime(string clock, out string infoText)
        {
            // Input checks:
            infoText = null;
            int clockTime;
            if (clock == null || clock.Length != 4 || !Int32.TryParse(clock, out clockTime))
            {
                infoText = "Input not in valid form.";
                return Task.FromResult(false);
            }

            if (lastUpdate != null && lastUpdate.AddSeconds(10) > DateTime.Now)
            {
                infoText = "Wait untill last update has been processed.";
                return Task.FromResult(false);
            }

            // Check if a valid time:
            char[] nums = clock.ToCharArray();
            List<int> values = new List<int>();
            for (int i = 0; i < nums.Length; i++)
            {
                int v;
                char c = nums[i];
                if (Int32.TryParse(c.ToString(), out v) && v <= 9 && 0 <= v)
                {
                    values.Add(v);
                }
                else
                {
                    infoText = "Input not in valid form.";
                    return Task.FromResult(false);
                }

                switch (i)
                {
                    case 0:
                        if (v > 2)
                        {
                            infoText = "Input not in valid form.";
                            return Task.FromResult(false);
                        }
                        break;
                    case 1:
                        if (nums[0] == '2')
                        {
                            if (v > 3) { infoText = "Input not in valid form."; return Task.FromResult(false); }
                        }
                        break;
                    case 2:
                        if (v > 5) { infoText = "Input not in valid form."; return Task.FromResult(false); }
                        break;
                    default:
                        break;
                }
            }



            string ip;
            if (Application.Current.Properties.ContainsKey("IP"))
                ip = (string)Application.Current.Properties["IP"];
            else
            {
                infoText = "No IP address selected.";
                return Task.FromResult(false);
            }
                

            // Time can be sent:
            var uri = ($"http://{ip}/{clock}ready");
            if (Communication.PostData(uri, 800, out infoText))
            {
                lastUpdate = DateTime.Now;
                return Task.FromResult(true);
            }
            return Task.FromResult(false);

        }

    }
}
