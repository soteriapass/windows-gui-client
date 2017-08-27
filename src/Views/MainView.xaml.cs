using System;
using System.Windows;

namespace PasswordManager
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainView
    {
        #region Variables

        private readonly MainViewModel _ViewModel;

        #endregion

        #region Ctor

        public MainView()
        {
            try
            {
                _ViewModel = new MainViewModel(this);
                DataContext = _ViewModel;

                InitializeComponent();
            }
            catch(Grpc.Core.RpcException ex)
            {
                if(ex.Status.StatusCode == Grpc.Core.StatusCode.Unavailable)
                {
                    MessageBox.Show("The remote service is currently unavailable", "Could not connect", MessageBoxButton.OK, MessageBoxImage.Error);
                }
                else
                {
                    MessageBox.Show($"An unknown communications error was encountered.\r\n\r\n{ex.Message}", "Could not connect", MessageBoxButton.OK, MessageBoxImage.Error);
                }
                throw;
            }
            catch(Exception ex)
            {
                MessageBox.Show($"An unknown error was detected\r\n{ex.Message}", "Unknown Error", MessageBoxButton.OK, MessageBoxImage.Error);
                throw;
            }
        }

        #endregion
    }
}
