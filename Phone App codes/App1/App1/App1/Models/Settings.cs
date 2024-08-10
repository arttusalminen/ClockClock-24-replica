using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Text;
using Xamarin.Forms;

namespace App1.Models
{
    public class Settings : INotifyPropertyChanged
    {
        #region INotifyPropertyChanged Members  

        public event PropertyChangedEventHandler PropertyChanged;
        private void OnPropertyChanged(string propertyName)
        {
            if (PropertyChanged != null)
            {
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }
        #endregion

        private string _ipAddress;

        public string IpAddress
        {
            get { return _ipAddress; }
            set 
            {
                _ipAddress = value;
                if (Application.Current.Properties.ContainsKey("IP"))
                    Application.Current.Properties.Remove("IP");
                Application.Current.Properties.Add("IP", value);
                OnPropertyChanged("IpAddress");
                Application.Current.SavePropertiesAsync();
            }
        }

        private static readonly Lazy<Settings> _settings = new Lazy<Settings>(() => new Settings());
        private Settings() 
        {
            if (Application.Current.Properties.ContainsKey("IP"))
                IpAddress = (string) Application.Current.Properties["IP"];
            else
                IpAddress = string.Empty;
        }
        public static Settings Instance
        {
            get { return _settings.Value; }
        }


        // UpdateSettings




    }
}
