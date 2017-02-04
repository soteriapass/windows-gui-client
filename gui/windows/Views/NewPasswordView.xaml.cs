namespace PasswordManager
{
    /// <summary>
    /// Interaction logic for NewPasswordView.xaml
    /// </summary>
    public partial class NewPasswordView
    {
        #region Variables

        private readonly NewPasswordViewModel _ViewModel;

        #endregion

        #region Ctor

        public NewPasswordView(Pswmgr.PasswordEntry entry = null)
        {
            _ViewModel = new NewPasswordViewModel(this, entry);
            DataContext = _ViewModel;

            InitializeComponent();

            _PasswordBox.Password = _ViewModel.Model.Password;
            Title = "Modify Password";
            _AccountNameTextBox.IsEnabled = entry == null;
        }

        #endregion

        #region Properties

        internal NewPasswordViewModel ViewModel
        {
            get { return _ViewModel; }
        }

        #endregion

        #region Methods

        private void PasswordBox_PasswordChanged(object sender, System.Windows.RoutedEventArgs e)
        {
            _ViewModel.OnPasswordBoxPasswordChanged();
        }

        #endregion
    }
}