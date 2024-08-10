using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Xamarin.Forms;
using Xamarin.Forms.Xaml;
using App1.Models;
using System.Windows.Input;
using System.Reflection;

namespace App1
{
    [XamlCompilation(XamlCompilationOptions.Compile)]
    public partial class SettingsPage : ContentPage
    {
        public SettingsPage()
        {
            InitializeComponent();

            IPCommand = new Command(IPCom);
            SyncCommand = new Command(SyncCom);
            UnSyncCommand = new Command(UnSyncCom);
            GradualCommand = new Command(GradualCom);
            SimultaneousCommand = new Command(SimultaneousCom);
            GradualIntervalCommand = new Command(GradIntCom);
            LapsCommand = new Command(LapsCom);
            AccCommand = new Command(AccCom);
            StepCommand = new Command(StepCom);



            this.BindingContext = this;
        }


        #region Commands
        

        #endregion


        private Settings _settings = Settings.Instance;


        public string CurrentIP
        {
            get => _settings.IpAddress;
            set
            {
                if (_settings.IpAddress != value)
                {
                    _settings.IpAddress = value;
                    if (Application.Current.Properties.ContainsKey("IP"))
                        Application.Current.Properties.Remove("IP");
                    Application.Current.Properties.Add("IP", value);
                    OnPropertyChanged();
                    Application.Current.SavePropertiesAsync();
                }
            }
        }



        private Xamarin.Forms.Color _syncColor = Xamarin.Forms.Color.White;
        public Color SyncColor
        {
            get { return _syncColor; }
            set
            {
                if (_syncColor != value)
                {
                    _syncColor = value;
                    OnPropertyChanged();
                }
            }
        }

        private Color _lapsColor = Color.White;
        public Color LapsColor
        {
            get { return _lapsColor; }
            set
            {
                if (_lapsColor != value)
                {
                    _lapsColor = value;
                    OnPropertyChanged();
                }
            }
        }

        private Color _unSyncColor = Color.White;
        public Color UnSyncColor
        {
            get { return _unSyncColor; }
            set
            {
                if (_unSyncColor != value)
                {
                    _unSyncColor = value;
                    OnPropertyChanged();
                }
            }
        }

        private Color _gradualColor = Color.White;
        public Color GradualColor
        {
            get { return _gradualColor; }
            set
            {
                if (_gradualColor != value)
                {
                    _gradualColor = value;
                    OnPropertyChanged();
                }
            }
        }

        private Color _simultaneousColor = Color.White;
        public Color SimultaneousColor
        {
            get { return _simultaneousColor; }
            set
            {
                if (_simultaneousColor != value)
                {
                    _simultaneousColor = value;
                    OnPropertyChanged();
                }
            }
        }

        private Color _gradualIntervalButColor = Color.White;
        public Color GradualIntervalButColor
        {
            get { return _gradualIntervalButColor; }
            set
            {
                if (_gradualIntervalButColor != value)
                {
                    _gradualIntervalButColor = value;
                    OnPropertyChanged();
                }
            }
        }



        private Color _ipButColor = Color.White;
        public Color IpButColor
        {
            get { return _ipButColor; }
            set
            {
                if (_ipButColor != value)
                {
                    _ipButColor = value;
                    OnPropertyChanged();
                }
            }
        }

        private Color _accButColor = Color.White;
        public Color AccButColor
        {
            get { return _accButColor; }
            set
            {
                if (_accButColor != value)
                {
                    _accButColor = value;
                    OnPropertyChanged();
                }
            }
        }

        private Color _stepButColor = Color.White;
        public Color StepButColor
        {
            get { return _stepButColor; }
            set
            {
                if (_stepButColor != value)
                {
                    _stepButColor = value;
                    OnPropertyChanged();
                }
            }
        }


        private string _entryGradualInterval;
        public string EntryGradualInterval
        {
            get { return _entryGradualInterval; }
            set
            {
                if (_entryGradualInterval != value)
                {
                    _entryGradualInterval = value;
                    OnPropertyChanged();
                }
            }
        }


        private string entryIPValue;
        public string EntryIPValue
        {
            get { return entryIPValue; }
            set
            {
                if (entryIPValue != value)
                {
                    entryIPValue = value;
                    OnPropertyChanged();
                }
            }
        }


        private string _lapsEntry;
        public string LapsEntry
        {
            get { return _lapsEntry; }
            set
            {
                if (_lapsEntry != value)
                {
                    _lapsEntry = value;
                    OnPropertyChanged();
                }
            }
        }


        private string _entryAccValue;
        public string EntryAccValue
        {
            get { return _entryAccValue; }
            set
            {
                if (_entryAccValue != value)
                {
                    _entryAccValue = value;
                    OnPropertyChanged();
                }
            }
        }

        private string _entryStepValue;
        public string EntryStepValue
        {
            get { return _entryStepValue; }
            set
            {
                if (_entryStepValue != value)
                {
                    _entryStepValue = value;
                    OnPropertyChanged();
                }
            }
        }


        public async Task Execute(Action<string> action, string nameof, int timeoutInMilliseconds)
        {
            await Task.Delay(timeoutInMilliseconds);
            action(nameof);
        }

        public void ResetButColors(string name)
        {
            this.GetType().GetProperty(name).SetValue(this, Color.White, null);
        }


        #region Commands
        public ICommand IPCommand { get; }
        public ICommand SyncCommand { get; }
        public ICommand UnSyncCommand { get; }
        public ICommand GradualCommand { get; }
        public ICommand SimultaneousCommand { get; }
        public ICommand GradualIntervalCommand { get; }
        public ICommand LapsCommand { get; }
        public ICommand AccCommand { get; }
        public ICommand StepCommand { get; }

        


        void IPCom() { Task.Run(() => IPChangedClick()); }
        void SyncCom() { Task.Run(() => SyncClick()); }
        void UnSyncCom() { Task.Run(() => UnSyncClick()); }
        void GradualCom() { Task.Run(() => GradualClick()); }
        void SimultaneousCom() { Task.Run(() => SimultaneousClick()); }
        void GradIntCom() { Task.Run(() => GradualIntervalClick()); }
        void LapsCom() { Task.Run(() => LapsClick()); }
        void AccCom() { Task.Run(() => AccChangedClick()); }
        void StepCom() { Task.Run(() => StepChangedClick()); }



        #endregion






        private async Task SyncClick()
        {
            // Check IP:
            if (CurrentIP == String.Empty)
            {
                SyncColor = Color.Red;
                Execute(ResetButColors, "SyncColor", 3000);
                return;
            }


            string data = $"syncOn";

            var uri = ($"http://{CurrentIP}/{data}ready");
            if (Communication.PostData(uri, 800, out _))
            {
                SyncColor = Color.Green;
                Execute(ResetButColors, "SyncColor", 3000);
            }
            else
            {
                SyncColor = Color.Red;
                Execute(ResetButColors, "SyncColor", 3000);
            }
        }
        private async Task UnSyncClick()
        {
            // Check IP:
            if (CurrentIP == String.Empty)
            {
                UnSyncColor = Color.Red;
                Execute(ResetButColors, "UnSyncColor", 3000);
                return;
            }

            string data = $"syncOff";

            var uri = ($"http://{CurrentIP}/{data}ready");
            if (Communication.PostData(uri, 800, out _))
            {
                UnSyncColor = Color.Green;
                Execute(ResetButColors, "UnSyncColor", 3000);
            }
            else
            {
                UnSyncColor = Color.Red;
                Execute(ResetButColors, "UnSyncColor", 3000);
            }
        }
        private async Task GradualClick()
        {
            // Check IP:
            if (CurrentIP == String.Empty)
            {
                GradualColor = Color.Red;
                Execute(ResetButColors, "GradualColor", 3000); 
                return;
            }

            string data = $"gradualOn";

            var uri = ($"http://{CurrentIP}/{data}ready");
            if (Communication.PostData(uri, 800, out _))
            {
                GradualColor = Color.Green;
                Execute(ResetButColors, "GradualColor", 3000);
            }
            else
            {
                GradualColor = Color.Red;
                Execute(ResetButColors, "GradualColor", 3000);
            }
        }
        private async Task SimultaneousClick()
        {
            // Check IP:
            if (CurrentIP == String.Empty)
            {
                SimultaneousColor = Color.Red;
                Execute(ResetButColors, "SimultaneousColor", 3000);
                return;
            }

            string data = $"gradualOff";

            var uri = ($"http://{CurrentIP}/{data}ready");
            if (Communication.PostData(uri, 800, out _))
            {
                SimultaneousColor = Color.Green;
                Execute(ResetButColors, "SimultaneousColor", 3000);
            }
            else
            {
                SimultaneousColor = Color.Red;
                Execute(ResetButColors, "SimultaneousColor", 3000);
            }
        }
        private async Task GradualIntervalClick()
        {
            // Check IP:
            if (CurrentIP == String.Empty)
            {
                GradualIntervalButColor = Color.Red;
                Execute(ResetButColors, "GradualIntervalButColor", 3000);
                return;
            }

            string data = $"gradual:{EntryGradualInterval}";

            var uri = ($"http://{CurrentIP}/{data}ready");
            if (Communication.PostData(uri, 800, out _))
            {
                GradualIntervalButColor = Color.Green;
                Execute(ResetButColors, "GradualIntervalButColor", 3000);
            }
            else
            {
                GradualIntervalButColor = Color.Red;
                Execute(ResetButColors, "GradualIntervalButColor", 3000);
            }
            EntryGradualInterval = "";
        }
        private async Task LapsClick()
        {
            // Check IP:
            if (CurrentIP == String.Empty)
            {
                LapsColor = Color.Red;
                Execute(ResetButColors, "LapsColor", 3000);
                return;
            }

            string data = $"laps:{LapsEntry}";

            var uri = ($"http://{CurrentIP}/{data}ready");
            if (Communication.PostData(uri, 800, out _))
            {
                LapsColor = Color.Green;
                Execute(ResetButColors, "LapsColor", 3000);
            }
            else
            {
                LapsColor = Color.Red;
                Execute(ResetButColors, "LapsColor", 3000);
            }
            LapsEntry = "";
        }






        private async Task IPChangedClick()
        {
            if (EntryIPValue is null)
            {
                IpButColor = Color.Red;
                Execute(ResetButColors, "IpButColor", 3000);
                return;
            }

            if (Application.Current.Properties.ContainsKey("IP"))
                Application.Current.Properties.Remove("IP");

            Application.Current.Properties.Add("IP", EntryIPValue);

            CurrentIP = EntryIPValue;
            EntryIPValue = null;
            Application.Current.SavePropertiesAsync();
            IpButColor = Color.Green;
            Execute(ResetButColors, "IpButColor", 3000);
        }

        private async Task AccChangedClick()
        {
            // Check that the value is int:
            if (!Int32.TryParse(EntryAccValue, out _))
            {
                AccButColor = Color.Red;
                Execute(ResetButColors, "AccButColor", 3000);
                return;
            }

            // Check IP:
            if (CurrentIP == String.Empty)
            {
                AccButColor = Color.Red;
                Execute(ResetButColors, "AccButColor", 3000);
                return;
            }


            string data = $"acceleration:{EntryAccValue}";

            var uri = ($"http://{CurrentIP}/{data}ready");
            if (Communication.PostData(uri, 800, out _))
            {
                AccButColor = Color.Green;
                Execute(ResetButColors, "AccButColor", 3000);
            }
            else
            {
                AccButColor = Color.Red;
                Execute(ResetButColors, "AccButColor", 3000);
            }
            EntryAccValue = "";
        }

        private async Task StepChangedClick()
        {
            // Check that the value is int:
            if (!Int32.TryParse(EntryStepValue, out _))
            {
                StepButColor = Color.Red;
                Execute(ResetButColors, "StepButColor", 3000);
                return;
            }

            // Check IP:
            if (CurrentIP == String.Empty)
            {
                StepButColor = Color.Red;
                Execute(ResetButColors, "StepButColor", 3000);
                return;
            }

            string data = $"stepinterval:{EntryStepValue}";

            var uri = ($"http://{CurrentIP}/{data}ready");
            if (Communication.PostData(uri, 800, out _))
            {
                StepButColor = Color.Green;
                Execute(ResetButColors, "StepButColor", 3000);
            }
            else
            {
                StepButColor = Color.Red;
                Execute(ResetButColors, "StepButColor", 3000);
            }
            EntryStepValue = "";
        }
    }
}