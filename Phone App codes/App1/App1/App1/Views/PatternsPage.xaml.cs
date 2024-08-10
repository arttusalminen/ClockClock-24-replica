using App1.Models;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;
using Xamarin.CommunityToolkit.UI.Views;
using Xamarin.Forms;
using Xamarin.Forms.Xaml;

namespace App1
{
    [XamlCompilation(XamlCompilationOptions.Compile)]
    public partial class PatternsPage : ContentPage
    {
        public PatternsPage()
        {
            InitializeComponent();

            SetShape1Command = new Command(Click1);
            SetShape2Command = new Command(Click2);
            SetShape3Command = new Command(Click3);


            this.BindingContext = this;
        }

        #region Commands
        public ICommand SetShape1Command { get; }
        public ICommand SetShape2Command { get; }
        public ICommand SetShape3Command { get; }

        #endregion

        void Click1() { Task.Run(() => PointMiddleClick()); }
        void Click2() { Task.Run(() => WavesClick()); }
        void Click3() { Task.Run(() => DownClick()); }




        private Color _but1Color = Color.White;
        private Color _but2Color = Color.White;
        private Color _but3Color = Color.White;

        public Color But1Color
        {
            get { return _but1Color; }
            set {
                if (_but1Color != value)
                {
                    _but1Color = value;
                    OnPropertyChanged();
                }
                
            }
        }

        public Color But2Color
        {
            get { return _but2Color; }
            set
            {
                if (_but2Color != value)
                {
                    _but2Color = value;
                    OnPropertyChanged();
                }

            }
        }

        public Color But3Color
        {
            get { return _but3Color; }
            set
            {
                if (_but3Color != value)
                {
                    _but3Color = value;
                    OnPropertyChanged();
                }

            }
        }

        public async Task Execute(Action<string> action, string but, int timeoutInMilliseconds)
        {
            await Task.Delay(timeoutInMilliseconds);
            action(but);
        }

        public void ResetButColors(string name)
        {
            this.GetType().GetProperty(name).SetValue(this, Color.White, null);
        }







        private async Task PointMiddleClick()
        {
            // Get IP:
            string ip;
            if (Application.Current.Properties.ContainsKey("IP"))
                ip = (string)Application.Current.Properties["IP"];
            else
            {
                // Color
                But1Color = Color.Red;
                Execute(ResetButColors, "But1Color", 5000);
                return;
            }

            // Send:
            string data = "pattern:pointMiddle";
            var uri = ($"http://{ip}/{data}ready");
            if (Communication.PostData(uri, 800, out _))
            {
                // Color
                But1Color = Color.Green;
                Execute(ResetButColors, "But1Color", 5000);
                return;
            }
            else
            {
                // Color
                But1Color = Color.Red;
                Execute(ResetButColors, "But1Color", 5000);
                return;
            }

        }



        private async Task WavesClick()
        {
            // Get IP:
            string ip;
            if (Application.Current.Properties.ContainsKey("IP"))
                ip = (string)Application.Current.Properties["IP"];
            else
            {
                // Color
                But2Color = Color.Red;
                Execute(ResetButColors, "But2Color", 5000);
                return;
            }

            // Send:
            string data = "pattern:waves";
            var uri = ($"http://{ip}/{data}ready");
            if (Communication.PostData(uri, 800, out _))
            {
                // Color
                But2Color = Color.Green;
                Execute(ResetButColors, "But2Color", 5000);
            }
            else
            {
                // Color
                But2Color = Color.Red;
                Execute(ResetButColors, "But2Color", 5000);
            }

        }

        private async Task DownClick()
        {
            // Get IP:
            string ip;
            if (Application.Current.Properties.ContainsKey("IP"))
                ip = (string)Application.Current.Properties["IP"];
            else
            {
                // Color
                But3Color = Color.Red;
                Execute(ResetButColors, "But3Color", 5000);
                return;
            }

            // Send:
            string data = "pattern:pointDown";
            var uri = ($"http://{ip}/{data}ready");
            if (Communication.PostData(uri, 800, out _))
            {
                // Color
                But3Color = Color.Green;
                Execute(ResetButColors, "But3Color", 5000);
                return;
            }
            else
            {
                // Color
                But3Color = Color.Red;
                Execute(ResetButColors, "But3Color", 5000);
                return;
            }

        }

    }
}