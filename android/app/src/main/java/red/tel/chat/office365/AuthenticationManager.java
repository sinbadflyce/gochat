package red.tel.chat.office365;

import android.annotation.TargetApi;
import android.app.Activity;
import android.app.PendingIntent;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.support.annotation.ColorRes;
import android.support.annotation.Nullable;
import android.util.Base64;
import android.util.Log;

import com.google.gson.Gson;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;

import net.openid.appauth.AuthState;
import net.openid.appauth.AuthorizationException;
import net.openid.appauth.AuthorizationRequest;
import net.openid.appauth.AuthorizationResponse;
import net.openid.appauth.AuthorizationService;
import net.openid.appauth.AuthorizationServiceConfiguration;
import net.openid.appauth.ResponseTypeValues;
import net.openid.appauth.TokenRequest;
import net.openid.appauth.TokenResponse;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;

import red.tel.chat.R;
import red.tel.chat.ui.LoginActivity;


public class AuthenticationManager {
    private AuthorizationServiceConfiguration mConfig;
    private AuthorizationRequest mAuthorizationRequest;
    private AuthState mAuthState;
    private AuthorizationService mAuthorizationService;

    private static final String TAG = "AuthenticationManager";
    private static AuthenticationManager INSTANCE;

    private Activity mContextActivity;
    private String mAccessToken;

    private AuthenticationManager() {
        Uri authorityUrl = Uri.parse(Constants.AUTHORITY_URL);
        Uri authorizationEndpoint = Uri.withAppendedPath(authorityUrl, Constants.AUTHORIZATION_ENDPOINT);
        Uri tokenEndpoint = Uri.withAppendedPath(authorityUrl, Constants.TOKEN_ENDPOINT);
        mConfig = new AuthorizationServiceConfiguration(authorizationEndpoint, tokenEndpoint, null);

        List<String> scopes = new ArrayList<>(Arrays.asList(Constants.SCOPES.split(" ")));

        mAuthorizationRequest = new AuthorizationRequest.Builder(
                mConfig,
                Constants.CLIENT_ID,
                ResponseTypeValues.CODE,
                Uri.parse(Constants.REDIRECT_URI))
                .setScopes(scopes)
                .build();
    }

    /**
     * Starts the authorization flow, which continues to net.openid.appauth.RedirectReceiverActivity
     * and then to ConnectActivity
     */
    public void startAuthorizationFlow() {
        Intent intent = new Intent(mContextActivity, LoginActivity.class);
        intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        PendingIntent redirectIntent = PendingIntent.getActivity(mContextActivity, mAuthorizationRequest.hashCode(), intent, 0);

        mAuthorizationService.performAuthorizationRequest(
                mAuthorizationRequest,
                redirectIntent,
                mAuthorizationService.createCustomTabsIntentBuilder()
                        .setToolbarColor(getColorCompat(R.color.colorPrimary))
                        .build());
    }

    public void processAuthorizationCode(Intent redirectIntent, final AuthorizationService.TokenResponseCallback callback) {
        AuthorizationResponse authorizationResponse = AuthorizationResponse.fromIntent(redirectIntent);
        AuthorizationException authorizationException = AuthorizationException.fromIntent(redirectIntent);
        mAuthState = new AuthState(authorizationResponse, authorizationException);

        if (authorizationResponse != null) {
            HashMap<String, String> additionalParams = new HashMap<>();
            TokenRequest tokenRequest = authorizationResponse.createTokenExchangeRequest(additionalParams);

            mAuthorizationService.performTokenRequest(
                    tokenRequest,
                    new AuthorizationService.TokenResponseCallback() {
                        @Override
                        public void onTokenRequestCompleted(
                                @Nullable TokenResponse tokenResponse,
                                @Nullable AuthorizationException ex) {
                            mAuthState.update(tokenResponse, ex);
                            if (tokenResponse != null) {
                                mAccessToken = tokenResponse.accessToken;
                            }
                            callback.onTokenRequestCompleted(tokenResponse, ex);
                        }
                    });
        } else {
            Log.i(TAG, "Authorization failed: " + authorizationException);
        }
    }

    public JsonObject getClaims(String idToken) {
        JsonObject retValue = null;
        String payload = idToken.split("[.]")[1];

        try {
            // The token payload is in the 2nd element of the JWT
            String jsonClaims = new String(Base64.decode(payload, Base64.DEFAULT), "UTF-8");
            JsonElement jelem = new Gson().fromJson(jsonClaims, JsonElement.class);
            retValue = jelem.getAsJsonObject();
        } catch ( IOException e) {
            Log.e(TAG, "Couldn't decode id token: " + e.getMessage());
        }
        return retValue;
    }

    /**
     * Disconnects the app from Office 365 by clearing the token cache, setting the client objects
     * to null, and removing the user id from shred preferences.
     */
    public void disconnect() {
        // Reset the AuthenticationManager object
        AuthenticationManager.resetInstance();
    }

    public static synchronized AuthenticationManager getInstance() {
        if (INSTANCE == null) {
            INSTANCE = new AuthenticationManager();
        }
        return INSTANCE;
    }

    private static synchronized void resetInstance() {
        INSTANCE = null;
    }

    /**
     * Set the context activity before connecting to the currently active activity.
     *
     * @param contextActivity Currently active activity which can be utilized for interactive
     *                        prompt.
     */
    public void setContextActivity(final Activity contextActivity) {
        mContextActivity = contextActivity;
        mAuthorizationService = new AuthorizationService(mContextActivity);
    }

    public void onDestroyService() {
        if (mAuthorizationService != null) {
            mAuthorizationService.dispose();
        }
    }

    /**
     * Returns the access token obtained in authentication
     *
     * @return mAccessToken
     */
    public String getAccessToken() throws TokenNotFoundException {
        if(mAccessToken == null) {
            throw new TokenNotFoundException();
        }
        return mAccessToken;
    }

    @TargetApi(Build.VERSION_CODES.M)
    @SuppressWarnings("deprecation")
    private int getColorCompat(@ColorRes int color) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            return mContextActivity.getColor(color);
        } else {
            return mContextActivity.getResources().getColor(color);
        }
    }
}
