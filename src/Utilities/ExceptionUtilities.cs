using System;

namespace PasswordManager.Utilities
{
    public static class ExceptionUtilities
    {
        public static void TryCatchIgnore(Action a)
        {
            try
            {
                a();
            }
            catch
            {

            }
        }

        public static T TryAssignCatchIgnore<T>(Func<T> a, T defaultReturn)
        {
            T returnValue = defaultReturn;
            try
            {
                returnValue = a();
            }
            catch
            {

            }
            return returnValue;
        }
    }
}
