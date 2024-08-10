using Stuff.Models;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;
using System.Timers;
using System.Windows.Input;
using Xamarin.Forms;
using Xamarin.Forms.Xaml;

namespace App1
{
    [XamlCompilation(XamlCompilationOptions.Compile)]
    public partial class TimePage :  ContentPage
    {
        public TimePage()
        {
            InitializeComponent();

            SetTimeCommand = new Command(OnSetTime, OnCanSetTime);

            BindingContext = this;
        }

        string entryvalue;
        string statusUpdate;
        Color statusTextColor = Color.White;


        public async Task Execute(Action action, int timeoutInMilliseconds)
        {
            await Task.Delay(timeoutInMilliseconds);
            action();
        }


        public string EntryVal
        {
            get { return entryvalue; }
            set
            {
                if (entryvalue != value)
                {
                    entryvalue = value;
                    OnPropertyChanged("EntryVal");
                }
            }
        }
        public string StatusUpdate
        {
            get { return statusUpdate; }
            set
            {
                if (statusUpdate != value)
                {
                    statusUpdate = value;
                    OnPropertyChanged("StatusUpdate");
                }
            }
        }
        public Color StatusTextColor
        {
            get { return statusTextColor; }
            set
            {
                if (statusTextColor != value)
                {
                    statusTextColor = value;
                    OnPropertyChanged("StatusTextColor");
                }
            }
        }

        public void HideStatusText()
        {
            StatusTextColor = Color.White;
        }

        #region Commands
        public ICommand SetTimeCommand { get; }
        

        #endregion

        void OnSetTime()
        {
            Task.Run(() => SetTimeClick());
        }
        bool OnCanSetTime()
        {
            return true;
        }

        // private async void SetTimeClick(object sender, EventArgs e) {
        private async Task SetTimeClick() {
            string val = EntryVal;
            EntryVal = "";
            Task<bool> t = Clock.Instance.UpdateClockTime(val, out string toDisplay);
            var success = t.Result;

            StatusUpdate = toDisplay;

            if (success)
                StatusTextColor = Color.Green;
            else
                StatusTextColor = Color.Red;

            StatusUpdate = toDisplay;

            Execute(HideStatusText, 10000);
        }



    }
}