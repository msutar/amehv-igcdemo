package android.igc.server;

import android.support.v7.app.ActionBarActivity;
import android.graphics.Color;
import android.os.AsyncTask;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;


public class MainActivity extends ActionBarActivity {

	public static Button mStartCommButton;
	public static TextView mDataRxTextView;
	public static Button mDataRxLabel;
	public static Button mSendData;
	private EditText mData2Send;
	MySyncTask asyncTask;
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		asyncTask=new MySyncTask ();
		mDataRxTextView = (TextView) findViewById(R.id.RxTextView);
		mDataRxLabel = (Button) findViewById(R.id.RxLabel);
		mData2Send = (EditText) findViewById(R.id.TxDataBox);
		mSendData = (Button) findViewById(R.id.SendData);
		mSendData.setClickable(false);
		mDataRxLabel.setClickable(false);
		mStartCommButton = (Button) findViewById(R.id.StartCommButton);
		mStartCommButton.setBackgroundColor(Color.BLUE);
		mStartCommButton.setOnClickListener(new View.OnClickListener() {
                                             public void onClick(View v) {
                                                            EstablishCommunication(v);
                                             };
        });
		mSendData = (Button) findViewById(R.id.SendData);
		mSendData.setOnClickListener(new View.OnClickListener() {
											public void onClick(View v) {
												SendData(v);
											};
		});

    }
    private void EstablishCommunication(View v)
    {
    	mDataRxTextView.setText("Waiting for Client Connection");
    	System.out.println("server............ Sending data from Activity .......... ");
    	if(!asyncTask.isCancelled())
    	{
    		asyncTask.cancel(true);
    	}
    	asyncTask=new MySyncTask ();
    	asyncTask.execute(1000);
    }
    
    private void SendData(View v)
    {
    	System.out.println("server............ Sending data from Activity .......... ");
    	String data = mData2Send.getText().toString();
    	Send2Client(data);
		MainActivity.mSendData.setBackgroundColor(Color.GREEN);
    }
    
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {

		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle action bar item clicks here. The action bar will
		// automatically handle clicks on the Home/Up button, so long
		// as you specify a parent activity in AndroidManifest.xml.
		int id = item.getItemId();
		if (id == R.id.action_settings) {
			return true;
		}
		return super.onOptionsItemSelected(item);
	}
	static {
		System.loadLibrary("connection-jni");
	}
	public native String Send2Client(String text);
}

// Separate thread to handle client read and write operations.

class MySyncTask extends AsyncTask<Integer, Integer, String>{
   
    private String message;
@Override
protected String doInBackground(Integer... params) {
	System.out.println("server: Running in Background Thread");
	System.out.println("server: Calling Native C++ code");
	startCommunication();
	return null;
	}
	public native String startCommunication();

    public String DisplayData(String text) {
    	System.out.println("server :............ DisplayData .......... ");
    	message = text;
    	publishProgress();
		return text;
    }
	protected void onProgressUpdate(Integer... progress) {
			System.out.println("server ............ onProgressUpdate .......... ");
			MainActivity.mDataRxTextView.setText(message);
			if(message.compareTo("Connection Established") != 0)
			{
				if(message.compareTo("Client Terminated") != 0)
				{
					MainActivity.mDataRxLabel.setBackgroundColor(Color.GREEN);
				}
				else
				{
					MainActivity.mDataRxLabel.setBackgroundColor(Color.LTGRAY);
					MainActivity.mSendData.setBackgroundColor(Color.LTGRAY);
				}
			}
			else
			{
				MainActivity.mDataRxLabel.setBackgroundColor(Color.LTGRAY);
			}
		}
	protected void onPostExecute(String result) {
			System.out.println("server ............ onPostExecute .......... ");
			MainActivity.mDataRxLabel.setBackgroundColor(Color.LTGRAY);
			MainActivity.mStartCommButton.setBackgroundColor(Color.LTGRAY);
			MainActivity.mSendData.setBackgroundColor(Color.LTGRAY);
			MainActivity.mStartCommButton.setClickable(true);
		}
	
	protected void onPreExecute() {
		System.out.println("server............ onPreExecute .......... ");
		MainActivity.mStartCommButton.setBackgroundColor(Color.GREEN);
		MainActivity.mStartCommButton.setClickable(false);
		MainActivity.mStartCommButton.setText("Server Active");
		MainActivity.mSendData.setClickable(true);
		}    
}
