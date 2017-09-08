﻿using Grpc.Core;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.IO;
using System.Runtime.CompilerServices;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;
using System.Linq;
using System;
using System.Collections.Generic;
using System.Security.Cryptography;
using PasswordManager.ObjectModel;

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

        private readonly ObservableCollection<PasswordWrapper> _Passwords;
        private readonly ObservableCollection<PasswordWrapper> _PasswordsRaw;

        private int _SelectedPasswordIndex;
        private string _SearchText;
        private bool _IsBusy;

        private readonly BusyScope _BusyScope;

        #endregion

        #region Ctor

        public MainViewModel(MainView view)
        {
            _View = view;
            _Model = App.Instance.Conf;

            _ConnectedStatus = "Disconnected";

            _Passwords = new ObservableCollection<PasswordWrapper>();
            _PasswordsRaw = new ObservableCollection<PasswordWrapper>();

            _SelectedPasswordIndex = -1;
            _BusyScope = new BusyScope(() => IsBusy = true, ()=> IsBusy = false);

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

        public ICommand ShowPasswordPermanently
        {
            get { return new DelegateCommand(ShowSelectedPasswordPermanently); }
        }

        public ICommand GeneratePassword
        {
            get { return new DelegateCommand(GeneratePasswordImpl); }
        }

        private void GeneratePasswordImpl()
        {
            const string alphanumericCharacters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890()`~!@#$%^&*-+=|\\{}[]:;\"\'<>,.?/";
            string password = GetRandomString(16, alphanumericCharacters);
            MessageBox.Show(_View, "New password generated and copied to the clipboard", "Password Generated", MessageBoxButton.OK, MessageBoxImage.Information);
            System.Windows.Clipboard.SetText(password);
        }

        //Code originally taken from http://stackoverflow.com/questions/54991/generating-random-passwords
        public static string GetRandomString(int length, IEnumerable<char> characterSet)
        {
            if (length < 0)
                throw new ArgumentException("length must not be negative", "length");
            if (length > int.MaxValue / 8) // 250 million chars ought to be enough for anybody
                throw new ArgumentException("length is too big", "length");
            if (characterSet == null)
                throw new ArgumentNullException("characterSet");
            var characterArray = characterSet.Distinct().ToArray();
            if (characterArray.Length == 0)
                throw new ArgumentException("characterSet must not be empty", "characterSet");

            var bytes = new byte[length * 8];
            new RNGCryptoServiceProvider().GetBytes(bytes);
            var result = new char[length];
            for (int i = 0; i < length; i++)
            {
                ulong value = BitConverter.ToUInt64(bytes, i * 8);
                result[i] = characterArray[value % (uint)characterArray.Length];
            }
            return new string(result);
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

        public ObservableCollection<PasswordWrapper> Passwords
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

        public bool IsBusy
        {
            get { return true; }
            set
            {
                if(_IsBusy != value)
                {
                    _IsBusy = value;
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

            using (_BusyScope.Start())
            {
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

            using (_BusyScope.Start())
            {
                Pswmgr.SimpleRequest request = new Pswmgr.SimpleRequest();
                var response = await _Client.ListPasswordsAsync(request);

                _PasswordsRaw.Clear();
                _Passwords.Clear();
                foreach (var password in response.Passwords)
                {
                    _PasswordsRaw.Add(new PasswordWrapper(password));
                    _Passwords.Clear(); 
                }
                Search(_SearchText);
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

            PasswordWrapper entry = _Passwords[_SelectedPasswordIndex];
            await _Client.DeletePasswordAsync(entry.WrappedItem);
            _Passwords.RemoveAt(_SelectedPasswordIndex);
        }

        private async void ModifyPassword()
        {
            if (_SelectedPasswordIndex == -1)
            {
                MessageBox.Show(_View, "No selected password", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                return;
            }

            PasswordWrapper entry = _Passwords[_SelectedPasswordIndex];

            NewPasswordView passwordView = new NewPasswordView(entry.WrappedItem)
            {
                Owner = _View
            };
            if (passwordView.ShowDialog() == true)
            {
                var response = await _Client.ModifyPasswordAsync(entry.WrappedItem);
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

            PasswordWrapper entry = _Passwords[_SelectedPasswordIndex];

            System.Windows.Clipboard.SetText(entry.Password);
        }

        private void ShowSelectedPasswordPermanently()
        {
            if (_SelectedPasswordIndex == -1)
            {
                MessageBox.Show(_View, "No selected password", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                return;
            }

            PasswordWrapper entry = _Passwords[_SelectedPasswordIndex];
        }

        private void Search(string searchTerm)
        {
            searchTerm = searchTerm?.ToLower();
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
                if(searchTerm == null)
                {
                    _Passwords.Add(entry);
                }
                else if(entry.AccountName.ToLower().Contains(searchTerm))
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
