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

        public NewPasswordView()
        {
            _ViewModel = new NewPasswordViewModel(this);
            DataContext = _ViewModel;

            InitializeComponent();
        }

        #endregion

        #region Properties

        internal NewPasswordViewModel ViewModel
        {
            get { return _ViewModel; }
        }

        #endregion

        private void PasswordBox_PasswordChanged(object sender, System.Windows.RoutedEventArgs e)
        {
            _ViewModel.OnPasswordBoxPasswordChanged();
        }
    }
}
