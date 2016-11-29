package ro.pub.cs.systems.eim.lab03.phonedialer.view;

import android.Manifest;
import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Bundle;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.Toast;

import ro.pub.cs.systems.eim.lab03.phonedialer.R;
import ro.pub.cs.systems.eim.lab03.phonedialer.general.Constants;

public class PhoneDialerActivity extends AppCompatActivity {

    private EditText phoneNumberEditText = null;

    private NumberButtonClickListener numberButtonClickListener = new NumberButtonClickListener();
    private BackspaceButtonClickListener backspaceButtonClickListener = new BackspaceButtonClickListener();
    private CallButtonClickListener callButtonClickListener = new CallButtonClickListener();
    private HangupButtonClickListener hangupButtonClickListener = new HangupButtonClickListener();
    private ContactsButtonClickListener contactsButtonClickListener = new ContactsButtonClickListener();

    private class NumberButtonClickListener implements View.OnClickListener {

        @Override
        public void onClick(View view) {
            phoneNumberEditText.setText(phoneNumberEditText.getText().toString() + ((Button)view).getText().toString());
        }
    }

    private class BackspaceButtonClickListener implements View.OnClickListener {

        @Override
        public void onClick(View view) {
            String phoneNumber = phoneNumberEditText.getText().toString();
            if (phoneNumber.length() > 0) {
                phoneNumber = phoneNumber.substring(0, phoneNumber.length() - 1);
                phoneNumberEditText.setText(phoneNumber);
            }
        }
    }

    private class CallButtonClickListener implements View.OnClickListener {

        @Override
        public void onClick(View view) {
            String phoneNumber = phoneNumberEditText.getText().toString();
            Intent intent = new Intent(Intent.ACTION_CALL);
            intent.setData(Uri.parse("tel:" + phoneNumber));
            if (ContextCompat.checkSelfPermission(getApplicationContext(), Manifest.permission.CALL_PHONE) == PackageManager.PERMISSION_GRANTED) {
                startActivity(intent);
            } else {
                ActivityCompat.requestPermissions((Activity) getApplicationContext(), new String[]{Manifest.permission.CALL_PHONE}, 1);
            }
        }
    }

    private class HangupButtonClickListener implements View.OnClickListener {

        @Override
        public void onClick(View view) {
            phoneNumberEditText.setText("");
            finish();
        }
    }

    private class ContactsButtonClickListener implements View.OnClickListener {

        @Override
        public void onClick(View view) {
            String phoneNumber = phoneNumberEditText.getText().toString();
            if (phoneNumber.length() > 0) {
                Intent intent = new Intent(Constants.CONTACTS_MANAGER_ACTIVITY);
                intent.putExtra(Constants.PHONE_NUMBER_KEY, phoneNumber);
                startActivityForResult(intent, Constants.CONTACTS_MANAGER_REQUEST_CODE);
            } else {
                Toast.makeText(getApplication(), getResources().getString(R.string.phone_number_error), Toast.LENGTH_LONG).show();
            }
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_phone_dialer);
        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        phoneNumberEditText = (EditText)findViewById(R.id.phone_number_edit_text);
        Button button;
        for (int index = 0; index < Constants.buttonIds.length; index++) {
            button = (Button)findViewById(Constants.buttonIds[index]);
            button.setOnClickListener(numberButtonClickListener);
        }

        ImageButton backspaceImageButton = (ImageButton)findViewById(R.id.backspace_button);
        backspaceImageButton.setOnClickListener(backspaceButtonClickListener);
        ImageButton callImageButton = (ImageButton)findViewById(R.id.call_button);
        callImageButton.setOnClickListener(callButtonClickListener);
        ImageButton hangupImageButton = (ImageButton)findViewById(R.id.hangup_button);
        hangupImageButton.setOnClickListener(hangupButtonClickListener);
        ImageButton contactsButton = (ImageButton)findViewById(R.id.contacts_button);
        contactsButton.setOnClickListener(contactsButtonClickListener);
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent intent) {
        switch (requestCode) {
            case Constants.CONTACTS_MANAGER_REQUEST_CODE:
                Toast.makeText(getApplication(), "Contacts Manager Activity returned with result " + resultCode, Toast.LENGTH_LONG).show();
                break;
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_phone_dialer, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }
}
