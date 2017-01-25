using Grpc.Core;
using Pswmgr;
using System.ComponentModel;
using System.IO;
using System.Runtime.CompilerServices;
using System.Windows;
using System.Windows.Input;

namespace PasswordManager
{
    class MainViewModel : INotifyPropertyChanged
    {
        #region Variables

        private readonly MainView _View;
        private readonly Conf _Model;

        private string _ConnectedStatus;

        #endregion

        #region Ctor

        public MainViewModel(MainView view)
        {
            _View = view;
            _Model = App.Instance.Conf;

            _ConnectedStatus = "Disconnected";

            Authenticate(null);
        }

        #endregion

        #region Properties

        public ICommand ShowOptions
        {
            get { return new ShowOptionsCommand(_View, this); }
        }

        public ICommand Exit
        {
            get { return new ExitCommand(); }
        }

        public string ConnectedStatus
        {
            get { return _ConnectedStatus; }
            private set
            {
                if(_ConnectedStatus != value)
                {
                    _ConnectedStatus = value;
                    OnPropertyChanged();
                }
            }
        }

        #endregion

        #region Methods

        public void Authenticate()
        {
            Authenticate(_View);
        }

        private void Authenticate(Window parentView)
        {
            if (!_Model.IsValid())
                return;

            if (!File.Exists(_Model.ServerCertificate))
                return;

            ConnectedStatus = "Connecting";

            string rootCertificate = File.ReadAllText(_Model.ServerCertificate);

            var creds = new SslCredentials(rootCertificate);
            Channel channel = new Channel(_Model.AuthenticationChannel, creds);

            AuthenticationRequest request = new AuthenticationRequest();
            var client = new Pswmgr.Authentication.AuthenticationClient(channel);

            if (string.IsNullOrEmpty(_Model.Username) || string.IsNullOrEmpty(_Model.Password))
            {
                LoginView view = new LoginView()
                {
                    Owner = parentView
                };
                if (view.ShowDialog() == true)
                {
                    request.Username = _Model.Username;
                    request.Password = "asdf";// _Model.Password; TODO: FIX THIS
                }
            }

            var result =  client.Authenticate(request);

            ConnectedStatus = "Authenticated";
        }

        #endregion

        #region Events

        public event PropertyChangedEventHandler PropertyChanged;

        protected void OnPropertyChanged([CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        #endregion
    }
}
