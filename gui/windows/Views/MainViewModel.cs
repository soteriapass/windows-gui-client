using Grpc.Core;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.IO;
using System.Runtime.CompilerServices;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;
using System.Linq;

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
        private readonly ObservableCollection<Pswmgr.PasswordEntry> _PasswordsRaw;

        private int _SelectedPasswordIndex;
        private string _SearchText;

        #endregion

        #region Ctor

        public MainViewModel(MainView view)
        {
            _View = view;
            _Model = App.Instance.Conf;

            _ConnectedStatus = "Disconnected";

            _Passwords = new ObservableCollection<Pswmgr.PasswordEntry>();
            _PasswordsRaw = new ObservableCollection<Pswmgr.PasswordEntry>();

            _SelectedPasswordIndex = -1;

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

        public ICommand AddPassword
        {
            get { return new NewPasswordCommand(_View, this); }
        }

        public ICommand RefreshPasswords
        {
            get
            {
                return new DelegateCommand(delegate
                {
                    FetchPasswords();
                });
            }
        }

        public ICommand Delete
        {
            get { return new DelegateCommand(DeletePassword); }
        }

        public ICommand Modify
        {
            get { return new DelegateCommand(ModifyPassword); }
        }

        public ICommand Copy
        {
            get { return new DelegateCommand(CopyPassword); }
        }

        public string ConnectedStatus
        {
            get { return _ConnectedStatus; }
            private set
            {
                if (_ConnectedStatus != value)
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

        public int SelectedPasswordIndex
        {
            get { return _SelectedPasswordIndex; }
            set
            {
                if (_SelectedPasswordIndex != value)
                {
                    _SelectedPasswordIndex = value;
                    OnPropertyChanged();
                }
            }
        }

        public string SearchText
        {
            get { return _SearchText; }
            set
            {
                if(_SearchText != value)
                {
                    _SearchText = value;
                    OnPropertyChanged();

                    Search(value);
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
                    request.Password = _Model.Password;
                }
            }

            var result = client.Authenticate(request);

            bool cancelled = false;
            while (result.TokenNeededFor2Fa && !cancelled)
            {
                LoginView view = new LoginView(true)
                {
                    Owner = parentView,
                    WindowStartupLocation = parentView == null ? WindowStartupLocation.CenterScreen : WindowStartupLocation.CenterOwner
                };
                if (view.ShowDialog() == true)
                {
                    request.Username = _Model.Username;
                    request.Password = _Model.Password;
                    request.TfaToken = int.Parse(_Model.TwoFactorAuthToken);
                }
                else
                {
                    cancelled = true;
                }
                result = client.Authenticate(request);
            }

            _Token = result.Token;

            ConnectedStatus = "Authenticated";

            CallCredentials callCreds = CallCredentials.FromInterceptor(CustomAuthProcessor);
            var compositeCreds = ChannelCredentials.Create(creds, callCreds);
            Channel passwordChannel = new Channel(_Model.PasswordManagerChannel, compositeCreds);
            _Client = new Pswmgr.PasswordManager.PasswordManagerClient(passwordChannel);
            FetchPasswords();
        }

        private async Task CustomAuthProcessor(AuthInterceptorContext context, Metadata metadata)
        {
            await Task.Run(() => metadata.Add("x-custom-auth-ticket", _Token));
            return;
        }

        private async void FetchPasswords()
        {
            if (_Client == null)
                return;

            Pswmgr.SimpleRequest request = new Pswmgr.SimpleRequest();
            var response = await _Client.ListPasswordsAsync(request);

            _Passwords.Clear();
            foreach (var password in response.Passwords)
            {
                _Passwords.Add(password);
            }
        }

        internal async Task<bool> AddNewPassword(Pswmgr.PasswordEntry newPassword)
        {
            var response = await _Client.AddPasswordAsync(newPassword);
            FetchPasswords();
            return response.Success;
        }

        private async void DeletePassword()
        {
            if (_SelectedPasswordIndex == -1)
            {
                MessageBox.Show(_View, "No selected password", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                return;
            }

            Pswmgr.PasswordEntry entry = _Passwords[_SelectedPasswordIndex];
            await _Client.DeletePasswordAsync(entry);
            _Passwords.RemoveAt(_SelectedPasswordIndex);
        }

        private async void ModifyPassword()
        {
            if (_SelectedPasswordIndex == -1)
            {
                MessageBox.Show(_View, "No selected password", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                return;
            }

            Pswmgr.PasswordEntry entry = _Passwords[_SelectedPasswordIndex];

            NewPasswordView passwordView = new NewPasswordView(entry)
            {
                Owner = _View
            };
            if (passwordView.ShowDialog() == true)
            {
                var response = await _Client.ModifyPasswordAsync(entry);
                if (response.Success)
                    MessageBox.Show(_View, "Modified password successfully", "Success", MessageBoxButton.OK, MessageBoxImage.Information);
                else
                    MessageBox.Show(_View, "Problem modifying the password", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }
            else
            {
                MessageBox.Show(_View, "Modification Cancelled", "Cancelled", MessageBoxButton.OK, MessageBoxImage.Exclamation);
            }
        }

        private void CopyPassword()
        {
            if (_SelectedPasswordIndex == -1)
            {
                MessageBox.Show(_View, "No selected password", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                return;
            }

            Pswmgr.PasswordEntry entry = _Passwords[_SelectedPasswordIndex];

            System.Windows.Clipboard.SetText(entry.Password);
        }

        private void Search(string searchTerm)
        {
            searchTerm = searchTerm.ToLower();
            if(!_PasswordsRaw.Any() && _Passwords.Any())
            {
                foreach(var entry in _Passwords)
                {
                    _PasswordsRaw.Add(entry);
                }
            }

            _Passwords.Clear();
            foreach(var entry in _PasswordsRaw)
            {
                if(entry.AccountName.ToLower().Contains(searchTerm))
                {
                    _Passwords.Add(entry);
                }
                else if(entry.Extra.ToLower().Contains(searchTerm))
                {
                    _Passwords.Add(entry);
                }
                else if (entry.Username.ToLower().Contains(searchTerm))
                {
                    _Passwords.Add(entry);
                }
                else if(LevenshteinDistance.Compute(entry.AccountName.ToLower(), searchTerm) < 5)
                {
                    _Passwords.Add(entry);
                }
                else if (LevenshteinDistance.Compute(entry.Extra.ToLower(), searchTerm) < 5)
                {
                    _Passwords.Add(entry);
                }
                else if (LevenshteinDistance.Compute(entry.Username.ToLower(), searchTerm) < 5)
                {
                    _Passwords.Add(entry);
                }
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
