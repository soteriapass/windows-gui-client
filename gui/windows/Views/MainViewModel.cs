using Grpc.Core;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.IO;
using System.Runtime.CompilerServices;
using System.Threading.Tasks;
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

        private Pswmgr.PasswordManager.PasswordManagerClient _Client;
        private string _Token;

        private readonly ObservableCollection<Pswmgr.PasswordEntry> _Passwords;

        #endregion

        #region Ctor

        public MainViewModel(MainView view)
        {
            _View = view;
            _Model = App.Instance.Conf;

            _ConnectedStatus = "Disconnected";

            _Passwords = new ObservableCollection<Pswmgr.PasswordEntry>();

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

        public ObservableCollection<Pswmgr.PasswordEntry> Passwords
        {
            get { return _Passwords; }
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

            Pswmgr.AuthenticationRequest request = new Pswmgr.AuthenticationRequest();
            var client = new Pswmgr.Authentication.AuthenticationClient(channel);

            if (string.IsNullOrEmpty(_Model.Username) || string.IsNullOrEmpty(_Model.Password))
            {
                LoginView view = new LoginView()
                {
                    Owner = parentView,
                    WindowStartupLocation = parentView == null ? WindowStartupLocation.CenterScreen : WindowStartupLocation.CenterOwner
                };
                if (view.ShowDialog() == true)
                {
                    request.Username = _Model.Username;
                    request.Password = "asdf";// _Model.Password; TODO: FIX THIS
                }
            }

            var result =  client.Authenticate(request);

            _Token = result.Token;

            ConnectedStatus = "Authenticated";

            CallCredentials callCreds = CallCredentials.FromInterceptor(CustomAuthProcessor);
            var compositeCreds = ChannelCredentials.Create(creds, callCreds);
            Channel passwordChannel = new Channel(_Model.PasswordManagerChannel, compositeCreds);
            _Client = new Pswmgr.PasswordManager.PasswordManagerClient(passwordChannel);
            OnPasswordClientCreated();
        }

        private async Task CustomAuthProcessor(AuthInterceptorContext context, Metadata metadata)
        {
            await Task.Run(() => metadata.Add("x-custom-auth-ticket", _Token));
            return;
        }

        private async void OnPasswordClientCreated()
        {
            if (_Client == null)
                return;

            Pswmgr.SimpleRequest request = new Pswmgr.SimpleRequest();
            var response = await _Client.ListPasswordsAsync(request);

            _Passwords.Clear();
            foreach(var password in response.Passwords)
            {
                _Passwords.Add(password);
            }
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
